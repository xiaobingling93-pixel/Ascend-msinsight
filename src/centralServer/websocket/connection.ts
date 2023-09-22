import { CONTENT_LENGTH_PREFIX, isResponse, PORT } from './defs';
import type { DataRequest, ModuleName, DataSource, Notification, Response, Request, ResponseHandler } from './defs';
import connector from '@/connection';

const createRequestHead = function (
    id: number,
    module: string,
    command: string,
    args: Request['params'],
    token?: string
): Request {
    return {
        id,
        moduleName: module,
        type: 'request',
        command,
        params: { token, ...args },
    };
};

export class Connection {
    private _ws: WebSocket | undefined;
    private _dataSource: DataSource;
    private _msgId: number = 0;
    private readonly _responseHandlers: Map<number, ResponseHandler> = new Map();
    private _token?: string;
    private _fetchFlag: boolean = true;

    constructor(dataSource: DataSource) {
        console.info('[connector]', 'init');
        if (this._ws !== undefined) {
            // wedge: close and release the old websocket
        }
        this._msgId = 0;
        this._ws = new WebSocket(`ws://${dataSource.remote}:${dataSource.port}`);
        this._dataSource = dataSource;
    }

    addDataPath(dataPath: string[] | string): void {
        !Array.isArray(dataPath) && (dataPath = [dataPath]);
        this._dataSource.dataPath.push(...dataPath);
    }

    deleteDataPath(dataPath: string[] | string): void {
        !Array.isArray(dataPath) && (dataPath = [dataPath]);
        this._dataSource.dataPath = this._dataSource.dataPath.filter(item => !dataPath.includes(item));
    }

    async reset(): Promise<void> {
        console.info('[connector]', 'reset');
    }

    disconnect(): void {
        if (!this._ws) {
            new Error('connection is not initialized');
            return;
        }
        this._ws.close()
    }

    async connect(): Promise<void> {
        this._fetchFlag = true;
        return new Promise((resolve, reject) => {
            if (!this._ws) {
                reject(new Error('connection is not initialized'));
                return;
            }
            this._ws.onopen = (ev: Event) => {
                console.info('[connector]', 'onopen');
                // token Create
                const msg: Request = createRequestHead(0, 'global', 'token.create', { token: '' });
                msg.params.deadTime = -1;
                this.request(msg);
            };

            this._ws.onmessage = (ev: MessageEvent<string>): void => {
                if (!ev.data.startsWith('Content-Length')) {
                    const dataObj = JSON.parse(ev.data);
                    if (dataObj.command === 'token.create') {
                        this._token = dataObj.body.token;
                        console.info('wsConnector', `handleTokenCreateMsg ${this._token}`);
                        resolve();
                    } else {
                        console.info('wsConnector', 'create token failure', 'warn');
                        reject(new Error('create token failure'));
                    }
                }
            };

            this._ws.onerror = (ev: Event): void => {
                console.error('[connector]', ev);
                reject(new Error('connect failed.'));
            };
        }) as Promise<void>;
    }

    async fetch(module: ModuleName, dataRequest: DataRequest): Promise<unknown> {
        if (this._ws === undefined) {
            return Promise.reject(new Error('connection not initialized'));
        }
        if (this._ws.onmessage !== null && this._fetchFlag) {
            this._fetchFlag = false;
            // message process func
            this._ws.onmessage = this.fetchDataOnMessage;
        }
        return new Promise((resolve, reject) => {
            const id = this._msgId++;
            const msg: Request = createRequestHead(
                id,
                module,
                dataRequest.command,
                dataRequest.params,
                this._token
            );
            this.request(msg);
            const reqCallback = (res: Response): void => {
                console.info('[connector]', 'received:', res);
                if (res.result && res.body !== undefined) {
                    // wedge: return cache resolve
                    resolve(res.body);
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

    fetchDataOnMessage = (ev: MessageEvent<string>): void => {
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
        if (!isResponse(msg)) {
            msg.body.dataSource = this._dataSource;
            connector.send(msg);
            return;
        }

        // handle response
        const reqId = msg.requestId;
        const callback = this._responseHandlers.get(reqId);
        if (callback === undefined) {
            console.warn(`handler for msg #${reqId} not found`);
            return;
        }
        this._responseHandlers.delete(reqId);
        callback(msg);
    };

    private async request(msg: Request): Promise<void> {
        const msgStr = JSON.stringify(msg);
        if (this._ws === undefined) {
            throw new Error('');
        }
        this._ws.send(`${CONTENT_LENGTH_PREFIX}:${new TextEncoder().encode(msgStr).length}\r\n\r\n`);
        this._ws.send(msgStr);
    }

    get isConnected(): boolean {
        return this._ws?.readyState === WebSocket.OPEN;
    }

    async findServerPort(): Promise<number> {
        // wedge: implement the true logic
        return Promise.resolve(PORT);
    }
}
