import { CONTENT_LENGTH_PREFIX, isResopnse, NotificationHandler, ResponseHandler, Request, Response, Notification, PORT } from './defs';

export class InsightConnection {
    private _ws: WebSocket | undefined;
    private _msgId: number = 0;
    private readonly _responseHandlers: Map<number, ResponseHandler> = new Map();
    private readonly _notificationHandlers: Record<string, NotificationHandler>;

    constructor(notificationHandlers: Record<string, NotificationHandler>) {
        this._notificationHandlers = notificationHandlers;
    }

    async reset(): Promise<void> {
        console.log('[connector]', 'reset');
        if (this._ws !== undefined) {
            // wedge: close and release the old websocket
        }
        // reset the connector state
        this._msgId = 0;
        this._responseHandlers.clear();
        const port = await this.findServerPort();
        this._ws = new WebSocket(`ws://localhost:${port}`);
    }

    async connect(): Promise<void> {
        return new Promise((resolve, reject) => {
            if (!this._ws) {
                reject(new Error('connection is not initialized'));
                return;
            }
            this._ws.onopen = (ev: Event) => {
                console.log('[connector]', 'onopen');
                // wedge: request token?
                resolve();
            };

            this._ws.onmessage = (ev: MessageEvent<string>): void => {
                if (ev.data.startsWith(CONTENT_LENGTH_PREFIX)) {
                    // ignore this message
                    return;
                }
                let msg: Response | Notification;
                try {
                    msg = JSON.parse(ev.data);
                } catch {
                    console.warn('cannot parse json data:', ev);
                    return;
                }

                // handle notifications
                if (!isResopnse(msg)) {
                    const handler = this._notificationHandlers[msg.method];
                    if (handler === undefined) {
                        console.warn(`handler for method ${msg.method} not found`);
                        return;
                    }
                    handler(msg.params);
                    return;
                }

                // handle response
                const reqId = msg.id;
                const callback = this._responseHandlers.get(reqId);
                if (callback === undefined) {
                    console.warn(`handler for msg #${reqId} not found`);
                    return;
                }
                this._responseHandlers.delete(reqId);
                callback(msg);
            };

            this._ws.onerror = (ev: Event): void => {
                console.error('[connector]', ev);
            };
        });
    }

    async fetch(method: string, params: Record<string, unknown>): Promise<unknown> {
        if (this._ws === undefined) {
            return Promise.reject(new Error('connection not initialized'));
        }
        return new Promise((resolve, reject) => {
            const id = this._msgId++;
            if (params.isBinary === true) {
                this.requestBinary({
                    id,
                    params,
                    method,
                });
            } else {
                this.request({
                    id,
                    params,
                    method,
                });
            }
            const reqCallback = (res: Response): void => {
                console.log('【Request】', method, params);
                console.log('[connector]', 'received:', res);
                if (res.result !== undefined) {
                    // wedge: return cache resolve
                    resolve(res.result);
                } else {
                    if (res.error === undefined) {
                        throw new Error('malformed response');
                    }
                    const { code, message } = res.error;
                    console.error('[connector]', 'errorCode:', code, 'msg:', message);
                    reject(new Error(message));
                }
            };
            this._responseHandlers.set(id, reqCallback);
        });
    }

    private async request(msg: Request): Promise<void> {
        if (this._ws === undefined) {
            throw new Error('');
        }
        if (msg.params.isBinary === true) {
            const data: any = msg.params.data;
            this._ws.send(data);
        }
        const msgStr = JSON.stringify(msg);
        this._ws.send(`${CONTENT_LENGTH_PREFIX}:${msgStr.length}\r\n\r\n`);
        this._ws.send(msgStr);
    }

    async findServerPort(): Promise<number> {
        // wedge: implement the true logic
        return Promise.resolve(PORT);
    }

    private async requestBinary(msg: Request): Promise<void> {
        if (this._ws === undefined) {
            throw new Error('');
        }
        const { id, method, params } = msg;
        const paramsStr = JSON.stringify({ id, method, params: params.params }).padEnd(1000, ' ');
        const buffer: any = params.buffer;
        const blob = new Blob([ paramsStr, buffer ]);
        this._ws.send(`${CONTENT_LENGTH_PREFIX}:${blob.size}\r\n\r\n`);
        this._ws.send(blob);
    }
}
