/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { CONTENT_LENGTH_PREFIX, isResponse, PORT, LOCAL_HOST } from './defs';
import type { DataRequest, ModuleName, ConnectHost, Notification, Response, Request, ResponseHandler } from './defs';
import connector from '@/connection';
import { message as Message, Modal } from 'antd';
import { customConsole as console } from 'ascend-utils';
import { connectRemote } from '../server';
import { store } from '../../store';
import { runInAction } from 'mobx';

const createRequestHead = function (
    id: number,
    module: string,
    command: string,
    args: Request['params'],
): Request {
    const params = {};
    Object.assign(params, args);
    return {
        id,
        moduleName: module,
        type: 'request',
        command,
        fileId: (params as any).dbPath ?? '',
        projectName: store.sessionStore.activeSession?.activeDataSource?.projectName ?? '',
        params,
    };
};

const MAX_RESPONSE_HANDLERS = 1000;

export interface ErrorMsg {
    error: {
        code: number;
        message: string;
    };
};

export class Connection {
    private readonly _ws: WebSocket | undefined;
    private readonly _host: ConnectHost;
    private _msgId: number = 0;
    private readonly _responseHandlers: Map<number, ResponseHandler> = new Map();
    private _fetchFlag: boolean = true;

    constructor(initHost: ConnectHost) {
        console.info('[connector]', 'init');
        if (this._ws !== undefined) {
            // wedge: close and release the old websocket
        }
        this._msgId = 0;
        const session = store.sessionStore.activeSession;
        const protocol = `${window.location.protocol === 'https:' && window.location.host !== 'wry.localhost' ? 'wss:' : 'ws:'}//`;
        if (initHost.jupyterlabProxy) {
            const { host, search } = window.location;
            const path = `${window.location.pathname.replace(/\/resources\/profiler\/frontend/, `/proxy/${initHost.port}/resources/profiler/frontend`)}`;
            const uri = protocol + host + path + search;
            this._ws = new WebSocket(uri);
            runInAction(() => {
                session.toIframeUrl = `${protocol}${host}${window.location.pathname.replace(/\/resources\/profiler\/frontend\/.*/, `/proxy/${initHost.port}`)}`;
            });
        } else if (!window.location.pathname.includes('/proxy/')) {
            const hostname = location.hostname && location.hostname !== '' ? location.hostname : LOCAL_HOST;
            let pathname = location.pathname && location.pathname !== '/' ? location.pathname.replace(/\/resources\/profiler\/frontend\/index.html/, '') : '';
            pathname = pathname.replace(/\/index.html/, '');
            this._ws = new WebSocket(`${protocol}${hostname}${pathname}:${initHost.port}${window.location.search}`);
            runInAction(() => {
                session.toIframeUrl = `${protocol}${hostname}${pathname}:${initHost.port}`;
            });
        } else {
            const { location } = window;
            const { host } = location;
            const path = `${window.location.pathname}`.replace(/proxy\/\d{4}/, `proxy/${initHost.port}`);
            const { search } = location;
            const uri = protocol + host + path + search;

            this._ws = new WebSocket(uri);
            runInAction(() => {
                session.toIframeUrl = `${protocol}${host}${path.replace(/\/index.html/, '')}`;
            });
        }
        this._host = initHost;
    }

    get isConnected(): boolean {
        return this._ws?.readyState === WebSocket.OPEN;
    }

    async reset(): Promise<void> {
        console.info('[connector]', 'reset');
    }

    disconnect(): void {
        if (!this._ws) {
            throw Error('connection is not initialized');
        }
        this._ws.close();
    }

    async connect(): Promise<void> {
        this._fetchFlag = true;
        return new Promise((resolve, reject) => {
            if (!this._ws) {
                reject(new Error('connection is not initialized'));
                return;
            }
            this._ws.onopen = (ev: Event): void => {
                console.info('[connector]', 'onopen');
                // 开始心跳检查
                this.initHeartCheck();
            };

            this._ws.onmessage = (ev: MessageEvent<string>): void => {
                resolve();
            };

            this._ws.onerror = (ev: Event): void => {
                Message.error('WebSocket connection failed! You are advised to restart MindStudio Insight.');
                console.error('[connector]', ev);
                reject(new Error('connect failed.'));
            };
            this._ws.onclose = (ev: Event): void => {
                Modal.warning({
                    content: 'WebSocket is already in CLOSING or CLOSED state! Please try to reconnect or restart MindStudio Insight.',
                    okText: 'Reconnect',
                    onOk: () => connectRemote(this._host),
                    closable: true,
                });
            };
        }) as Promise<void>;
    }

    async fetch<T>(module: ModuleName, dataRequest: DataRequest, voidResponse: boolean = false): Promise<T | ErrorMsg> {
        if (!this.isConnected) {
            Message.error('WebSocket is already in CLOSING or CLOSED state! You are advised to restart MindStudio Insight.');
        }
        if (this._ws === undefined) {
            return Promise.reject(new Error('connection not initialized'));
        }
        if (this._ws.onmessage !== null && this._fetchFlag) {
            this._fetchFlag = false;
            // message process func
            this._ws.onmessage = this.fetchDataOnMessage;
        }
        return new Promise((resolve: (v: T | ErrorMsg) => void, reject) => {
            const id = this._msgId++;
            const msg: Request = createRequestHead(
                id,
                module,
                dataRequest.command,
                dataRequest.params,
            );
            this.request(msg);
            const reqCallback = this.getCallback(resolve);
            if (voidResponse) {
                return;
            }
            if (this._responseHandlers.size > MAX_RESPONSE_HANDLERS) {
                const firstKey = this._responseHandlers.keys().next().value;
                this._responseHandlers.delete(firstKey);
            }
            this._responseHandlers.set(id, reqCallback);
        });
    }

    getCallback<T>(resolve: (p: T | ErrorMsg) => void): (res: Response) => void {
        return (res: Response): void => {
            if (res.result && res.body !== undefined) {
                // wedge: return cache resolve
                resolve(res.body as T);
            } else {
                if (res.error === undefined) {
                    throw new Error('malformed response');
                }
                const { code, message } = res.error;
                console.error('[connector]', 'errorCode:', code, 'msg:', message || res.message);
                resolve({ error: { code, message: message || res.message } } as ErrorMsg);
            }
        };
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
            msg.body.dataSource = this._host;
            connector.send({ ...msg });
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

    async findServerPort(): Promise<number> {
        // wedge: implement the true logic
        return Promise.resolve(PORT);
    }

    private async request(msg: Request): Promise<void> {
        const msgStr = JSON.stringify(msg);
        if (this._ws === undefined) {
            throw new Error('');
        }
        this._ws.send(msgStr);
    }

    private initHeartCheck(): void {
        this.sendHeartCheck();
        setTimeout(() => {
            this.initHeartCheck();
        }, 30 * 1000);
    }

    private sendHeartCheck(): void {
        const msg: Request = createRequestHead(this._msgId++, 'global', 'heartCheck', {});
        this.request(msg);
    }
}
