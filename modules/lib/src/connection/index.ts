/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { customConsole as console } from 'ascend-utils';
type ReservedEventHandler = 'request';
type EventHanlder = string;
interface SendParams<T extends EventHanlder> {
    [x: string]: unknown;
    to?: SendTargetKey;
    module?: string;
    event: T extends ReservedEventHandler ? never : T;
};
type ListenerCallback = (res: MessageEvent) => void;
interface ListenerHandler { event: EventHanlder; sequence: number };
type TargetWindow = Window;
type GetTragetWindows = () => TargetWindow[];
type SendTargetKey = number;
abstract class BaseConnector {
    protected readonly _errMsgType = 'postMessage';
    protected invalidFunc: null | (() => void) = null;
    protected _targetWindows: TargetWindow[] | undefined;
    protected _getTargetWindows: GetTragetWindows;
    protected _listeners: Map<EventHanlder, Array<ListenerCallback | null>> = new Map();

    constructor(getTargetWindow: GetTragetWindows) {
        if (typeof window !== 'object') {
            const errMsg = 'cannot find global Window object, please check your runtime environment';
            console.error(this.printErrMsg(errMsg));
            this.invalidFunc = (): void => { throw new Error(this.printErrMsg(errMsg)); };
        }
        this._getTargetWindows = getTargetWindow;

        window.onmessage = (event: MessageEvent): void => {
            if (!(typeof event.data === 'string' && event.data.includes('{'))) {
                return;
            }
            const res = { ...event };
            try {
                res.data = JSON.parse(event.data);
                const listener = this._listeners.get(res.data.event);
                if (res.data.event === 'request') {
                    this.awaitFetch(res);
                } else if (listener) {
                    listener.forEach(cb => cb?.(res));
                } else if (['mouseover'].includes(res.data.event)) {
                    // 鼠标事件，无操作
                } else {
                    console.warn(this.printErrMsg('missed [event] in your message, please check your params, or maybe have an invalid send'));
                }
            } catch (e) {
                console.error(e);
            }
        };
        Object.defineProperty(window, 'onmessage', {
            configurable: false,
            set: () => {
                console.warn(this.printErrMsg('the property \'window.onmessage\' in used, reassign it is invalid'));
            },
        });
    }

    send<T extends EventHanlder>(body: SendParams<T>, reject?: (err: Error) => void): void {
        if (this.invalidFunc) {
            this.invalidFunc();
            return;
        }
        this._targetWindows = this._getTargetWindows();
        const isPostMessageInvalid = body.to !== undefined && window.parent !== null && window.parent[body.to]?.postMessage === undefined;
        if (isPostMessageInvalid || this._targetWindows.length === 0) {
            const errMsg = 'missed postMessage function, please check your iframe element';
            console.warn(this.printErrMsg(errMsg));
            reject?.(new Error(errMsg));
            return;
        }
        body.from = Object.entries(window.parent as Window).findIndex(([, val]) => val === window);
        const targetWindows = body.to !== undefined ? [(window.parent as Window)[body.to]] : this._targetWindows;
        const postBody = body.keepRawData === true ? body : JSON.stringify(body);
        targetWindows.forEach(targetWindow => {
            targetWindow.postMessage(postBody, this.getTargetOrigin());
        });
    }

    addListener<T extends EventHanlder>(event: T extends ReservedEventHandler ? never : T, callback: ListenerCallback): ListenerHandler {
        if (this.invalidFunc) {
            this.invalidFunc();
        }
        if (this._listeners.has(event)) {
            (this._listeners.get(event) as ListenerCallback[]).push(callback);
        } else {
            this._listeners.set(event, [callback]);
        }
        return { event, sequence: (this._listeners.get(event) as ListenerCallback[]).length - 1 };
    }

    removeListener({ event, sequence }: ListenerHandler): void {
        if (this.invalidFunc) {
            this.invalidFunc();
        }
        const listeners = this._listeners.get(event);
        if (listeners) {
            listeners[sequence] = null;
            if (listeners.filter(item => item).length === 0) {
                this._listeners.delete(event);
            }
        }
    }

    getTargetOrigin(): string {
        return process.env.NODE_ENV === 'development' || window.location.origin.startsWith('file') ? '*' : window.location.origin;
    }

    protected printErrMsg(errMsg: string): string {
        return `[${this._errMsgType}]: ${errMsg}`;
    }

    protected abstract awaitFetch(e: MessageEvent): void;
};

type FetchSequenceID = number;

interface FetchParams {
    [x: string]: unknown;
    event?: never;
};
interface FetchRequest {
    [x: string]: any;
    event: 'request';
    from: SendTargetKey;
    id: FetchSequenceID;
};
export class ClientConnector extends BaseConnector {
    private readonly _msgSequence: Map<FetchSequenceID, {resolve: (value: unknown) => void; reject: (value: unknown) => void}> = new Map();
    private _curFetchSequenceID: FetchSequenceID = 0;
    private readonly _module: string;

    constructor({
        getTargetWindow,
        module,
    }: {
        getTargetWindow: GetTragetWindows;
        module: string;
    }) {
        super(getTargetWindow);
        this._module = module;
    }

    send<T extends EventHanlder>(body: SendParams<T>, reject?: (err: Error) => void): void {
        body.module = body.module ?? this._module;
        super.send(body, reject);
    }

    async fetch(params: FetchParams): Promise<unknown> {
        return new Promise((resolve, reject) => {
            params.id = this._curFetchSequenceID;
            (params as Record<string, unknown>).event = 'request';
            const body = params as unknown as SendParams<string>;
            const voidResponse = params?.voidResponse as boolean;
            this.send(body, reject);
            if (voidResponse) {
                return;
            }
            this._msgSequence.set(this._curFetchSequenceID++, { resolve, reject });
        });
    };

    protected awaitFetch(res: MessageEvent<FetchRequest>): void {
        const result = this._msgSequence.get(res.data.id);
        if (result !== undefined) {
            const { resolve, reject } = result;
            if (res.data.body?.error !== null && res.data.body?.error !== undefined) {
                reject(res.data.body.error);
            } else {
                resolve(res.data);
            }
            this._msgSequence.delete(res.data.id);
        }
    }
};

type Response = (res: MessageEvent) => Promise<Record<string, unknown>>;
export class ServerConnector extends BaseConnector {
    private _responseForFetch: Response | null = null;
    private _getInterceptorHandlers: (command: string) => <T>(event: MessageEvent, responseInterceptor: T) => void;
    constructor({getTargetWindow, getInterceptorHandlers}: {
        getTargetWindow: GetTragetWindows;
        getInterceptorHandlers: (command: string) => <T>(event: MessageEvent, responseInterceptor: T) => void;
    }) {
        super(getTargetWindow);
        this._getInterceptorHandlers = getInterceptorHandlers;
    }

    resigsterAwaitFetch(callback: Response): void {
        this._responseForFetch = callback;
    }

    protected async awaitFetch(event: MessageEvent): Promise<void> {
        if (typeof event.data.id !== 'number' || !this._responseForFetch) {
            const errMsg = 'something wrong with requset response for fetch listener or data.id, please check your config';
            console.error(this.printErrMsg(errMsg));
            return;
        }
        const res = await this._responseForFetch(event);
        // 判断是否对该命令的返回内容进行了拦截配置，如果有配置，则先执行拦截方法逻辑
        const callback = this._getInterceptorHandlers(event.data.args.command);
        if (callback) {
            callback(event, res);
        }
        this.send({
            event: 'request',
            to: event.data.from,
            id: event.data.id,
            ...res,
        } as SendParams<EventHanlder>);
    }
}
