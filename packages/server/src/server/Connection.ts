import { Client, Handler, ShadowSession } from '../types';
import { WebSocket } from 'ws';
import { CONTENT_LENGTH_PREFIX, Request } from '../common/common';
import { InsightError } from '../utils/error';

export class Connection implements Client {
    private readonly _ws: WebSocket;
    private readonly _reqHandlerMap: Record<string, Handler>;
    private readonly _shadowSession: ShadowSession;

    // keeps the requests that are still being processed, mainly for debugging
    private readonly _unresolvedRequests: Map<number, Record<string, unknown>>;

    constructor(ws: WebSocket, handlers: Record<string, Handler>) {
        this._reqHandlerMap = handlers;
        this._unresolvedRequests = new Map();
        this._shadowSession = new ShadowSession();
        this._ws = ws;
        this._ws.on('error', console.error);
        this._ws.on('message', (data, isBinary) => {
            if (isBinary) {
                console.warn('unsupported binary message');
                return;
            }
            if (data.toString().startsWith(CONTENT_LENGTH_PREFIX)) {
                // ignore?
                return;
            }
            // wedge: what if json parsing error
            const req: Request = JSON.parse(data.toString());
            const handler = this._reqHandlerMap[req.method];
            if (handler === undefined) {
                this.error(req.id, 0, `handler for ${req.method} is not registered`);
            }
            if (this._unresolvedRequests.has(req.id)) {
                this.error(req.id, 1, 'duplicate request id');
            }
            this._unresolvedRequests.set(req.id, req);
            handler(req.params, this).then(result => {
                this.reply(req.id, result);
            }).catch(e => {
                if (e instanceof InsightError) {
                    this.error(req.id, e.code, e.message);
                } else {
                    this.error(req.id, 0, 'internal error');
                }
            }).finally(() => {
                this._unresolvedRequests.delete(req.id);
            });
        });
    }

    notify(method: string, data: Record<string, unknown>): void {
        console.log('notify', data);
        this.sendObj({
            method,
            params: data,
        });
    }

    get shadowSession(): ShadowSession {
        return this._shadowSession;
    }

    // todo: standardize error codes
    private error(id: number, code: number, message: string): void {
        console.log('error', id, code, message);
        this.sendObj({
            id,
            error: {
                code,
                message,
            },
        });
    }

    private reply(id: number, result: Record<string, unknown>): void {
        this.sendObj({
            id,
            result,
        });
    }

    private sendObj(obj: Record<string, unknown>): void {
        const responseStr = JSON.stringify(obj);
        this._ws.send(`${CONTENT_LENGTH_PREFIX}:${responseStr.length}\r\n\r\n`);
        this._ws.send(responseStr);
    }
}
