/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { CONTENT_LENGTH_PREFIX, isResponse, PORT, LOCAL_HOST } from './defs';
import type { DataRequest, ModuleName, DataSource, Notification, Response, Request, ResponseHandler } from './defs';
import connector from '@/connection';
import { ElMessage, ElMessageBox } from 'element-plus';
import { console } from '@/utils/console';
import { connectRemote } from '../server';
import { useDataSources } from '@/stores/dataSource';

const createRequestHead = function (
    id: number,
    module: string,
    command: string,
    args: Request['params'],
): Request {
    const { lastDataSource } = useDataSources();
    return {
        id,
        moduleName: module,
        type: 'request',
        command,
        projectName: lastDataSource.projectName,
        params: { ...args },
    };
};

const MAX_RESPONSE_HANDLERS = 1000;

export class Connection {
    private _ws: WebSocket | undefined;
    private _dataSource: DataSource;
    private _msgId: number = 0;
    private readonly _responseHandlers: Map<number, ResponseHandler> = new Map();
    private _fetchFlag: boolean = true;
    private _heartCheckTimer: any;

    constructor(dataSource: DataSource) {
        console.info('[connector]', 'init');
        if (this._ws !== undefined) {
            // wedge: close and release the old websocket
        }
        this._msgId = 0;

        let protocol = `${window.location.protocol === 'https:' && window.location.host !== 'wry.localhost' ? 'wss:' : 'ws:'}//`;
        if (!window.location.pathname.includes('\/proxy\/')) {
            const hostname = location.hostname && location.hostname !== '' ? location.hostname : LOCAL_HOST;
            this._ws = new WebSocket(`${protocol}${hostname}:${dataSource.port}`);
        } else {
            const { location } = window;
            const { host } = location;
            let path = `${window.location.pathname}`.replace(/proxy\/\d{4}/, `proxy/${dataSource.port}`);
            const { search } = location;
            let uri = protocol + host + path + search;

            this._ws = new WebSocket(uri);
        }
        this._dataSource = dataSource;
    }

    get isConnected(): boolean {
        return this._ws?.readyState === WebSocket.OPEN;
    }

    addDataPath(dataPath: string[] | string): void {
        let tempPath = dataPath;
        if (!Array.isArray(dataPath)) {
            tempPath = [dataPath];
        }
        this._dataSource.dataPath.push(...tempPath);
    }

    deleteDataPath(dataPath: string[] | string): void {
        let tempPath = dataPath;
        if (!Array.isArray(dataPath)) {
            tempPath = [dataPath];
        }
        this._dataSource.dataPath = this._dataSource.dataPath.filter(item => !tempPath.includes(item));
    }

    async reset(): Promise<void> {
        console.info('[connector]', 'reset');
    }

    disconnect(): void {
        if (!this._ws) {
            new Error('connection is not initialized');
            return;
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
                ElMessage.error('WebSocket connection failed! You are advised to restart MindStudio Insight.');
                console.error('[connector]', ev);
                reject(new Error('connect failed.'));
            };
            this._ws.onclose = (ev: Event): void => {
                ElMessageBox.alert('WebSocket is already in CLOSING or CLOSED state! Please try to reconnect or restart MindStudio Insight.', {
                    type: 'error',
                    confirmButtonText: 'Reconnect',
                }).then(() => {
                    connectRemote(this._dataSource);
                });
            };
        }) as Promise<void>;
    }

    async fetch(module: ModuleName, dataRequest: DataRequest, voidResponse: boolean = false): Promise<unknown> {
        if (!this.isConnected) {
            ElMessage.error('WebSocket is already in CLOSING or CLOSED state! You are advised to restart MindStudio Insight.');
        }
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
            );
            this.request(msg);
            const reqCallback = (res: Response): void => {
                if (res.result && res.body !== undefined) {
                    // wedge: return cache resolve
                    resolve(res.body);
                } else {
                    if (res.error === undefined) {
                        throw new Error('malformed response');
                    }
                    const { code, message } = res.error;
                    console.error('[connector]', 'errorCode:', code, 'msg:', message);
                    resolve({ error: res.error ?? {} });
                }
            };
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

    fetchDataOnMessage = (ev: MessageEvent<string>): void => {
        this.clearHeartCheckTimer();
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
            connector.send({...msg});
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
        this.setHeartCheck();
    }

    private initHeartCheck(): void {
        this.sendHeartCheck();
        setTimeout(() => {
            this.initHeartCheck();
        }, 30 * 1000);
    }

    private setHeartCheck(): void {
        this.clearHeartCheckTimer();
        this._heartCheckTimer = setTimeout(() => {
            this.sendHeartCheck();
        }, 60 * 1000);
    }

    private clearHeartCheckTimer(): void {
        if (this._heartCheckTimer) {
            clearTimeout(this._heartCheckTimer);
            this._heartCheckTimer = undefined;
        }
    }

    private sendHeartCheck(): void {
        const msg: Request = createRequestHead(this._msgId++, 'global', 'heartCheck', {});
        this.request(msg);
    }
}
