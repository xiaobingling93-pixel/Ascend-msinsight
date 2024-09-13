/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { console } from '@/utils/console';
import { INTERCEPTOR_HANDLERS, type ResponseType } from '@/connection/interceptor';
import { safeJSONParse } from '@/utils';

type ReservedEventHandler = 'request';
type EventHanlder = string;
interface SendParams<T extends EventHanlder> {
    [x: string]: unknown;
    to?: SendTargetKey;
    module?: TypeForConnector;
    event: T extends ReservedEventHandler ? never : T;
};
type SendTargetKey = number;
type ListenerCallback = (res: MessageEvent) => void;
interface ListenerHandler { event: EventHanlder; sequence: number };
type TargetWindow = Window;
type GetTragetWindows = () => TargetWindow[];
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
            this.invalidFunc = (): void => {
                throw new Error(this.printErrMsg(errMsg));
            };
        }
        this._getTargetWindows = getTargetWindow;

        window.onmessage = (event: MessageEvent): void => {
            const res = { ...event };
            if (typeof event.data === 'string') {
                res.data = safeJSONParse(event.data, {});
            } else {
                res.data = event.data;
            }
            const listener = this._listeners.get(res.data.event);
            if (res.data.event === 'request') {
                this.awaitFetch(res);
            } else if (listener) {
                listener.forEach((cb) => cb?.(res));
            } else {
                console.warn(this.printErrMsg('missed [event] in your message, please check your params, or maybe have an invalid send'));
            }
        };
        Object.defineProperty(window, 'onmessage', {
            configurable: false,
            set: () => {
                console.warn(this.printErrMsg("the property 'window.onmessage' in used, reassign it is invalid"));
            },
        });
    }

    send<T extends EventHanlder>(body: SendParams<T>, reject?: (value: unknown) => void): void {
        if (this.invalidFunc) {
            this.invalidFunc();
            return;
        }
        this._targetWindows = this._getTargetWindows();
        const isMissingPostMessage = body.to !== undefined && window !== null && window[body.to]?.postMessage === undefined;
        if (isMissingPostMessage || this._targetWindows.length === 0) {
            const errMsg = 'missed postMessage function, please check your iframe element';
            console.warn(this.printErrMsg(errMsg));
            reject?.(new Error(errMsg));
            return;
        }
        body.from = Object.entries(window as Window).findIndex(([, val]) => val === window);
        const targetWindows = body.to !== undefined ? [(window as Window)[body.to]] : this._targetWindows;
        targetWindows.forEach((targetWindow) => {
            targetWindow.postMessage(JSON.stringify(body), this.getTargetOrigin());
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
            if (listeners.filter((item) => item).length === 0) {
                this._listeners.delete(event);
            }
        }
    }

    getTargetOrigin(): string {
        return import.meta.env.DEV || window.location.origin.startsWith('file') ? '*' : window.location.origin;
    }

    protected printErrMsg(errMsg: string): string {
        return `[${this._errMsgType}]: ${errMsg}`;
    }

    protected abstract awaitFetch(e: MessageEvent): void;
}

type FetchSequenceID = number;
type Response = (res: MessageEvent) => Promise<Record<string, unknown>>;
class ServerConnector extends BaseConnector {
    private _responseForFetch: Response | null = null;

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
        const callback = INTERCEPTOR_HANDLERS[event.data.args.command];
        if (callback) {
            callback(event, res as unknown as ResponseType);
        }
        this.send({
            event: 'request',
            to: event.data.from,
            id: event.data.id,
            ...res,
        } as SendParams<EventHanlder>);
    }
}

interface FetchParams {
    [x: string]: unknown;
    event?: never;
}
interface FetchRequest {
    [x: string]: unknown;
    event: 'request';
    from: SendTargetKey;
    id: FetchSequenceID;
}
class ClientConnector extends BaseConnector {
    private readonly _msgSequence: Map<FetchSequenceID, (value: unknown) => void> = new Map();
    private _curFetchSequenceID: FetchSequenceID = 0;
    private readonly _module: TypeForConnector;

    constructor({ getTargetWindow, module }: { getTargetWindow: GetTragetWindows; module: TypeForConnector }) {
        super(getTargetWindow);
        this._module = module;
    }

    send<T extends EventHanlder>(body: SendParams<T>, reject?: (value: unknown) => void): void {
        body.module = body.module ?? this._module;
        super.send(body, reject);
    }

    async fetch(params: FetchParams): Promise<unknown> {
        return new Promise((resolve, reject) => {
            params.id = this._curFetchSequenceID;
            (params as Record<string, unknown>).event = 'request';
            const body = params as unknown as SendParams<string>;
            this.send(body, reject);
            this._msgSequence.set(this._curFetchSequenceID++, resolve);
        });
    }

    protected awaitFetch(res: MessageEvent<FetchRequest>): void {
        const _resolve = this._msgSequence.get(res.data.id);
        if (!_resolve) {
            console.warn(this.printErrMsg('cannot find relative resolve for this fetch, this maybe cause omission of message, please check your connection'));
            return;
        }
        _resolve(res.data);
        this._msgSequence.delete(res.data.id);
    }
}

type TypeForConnector = 'framework' | string;
type ConnectorType<T extends TypeForConnector> = T extends 'framework' ? ServerConnector : ClientConnector;
export default (function connectorFactory<T extends TypeForConnector>(connectorType: T): ConnectorType<T> {
    return (
        connectorType === 'framework'
            ? new ServerConnector(() => {
                  const res: TargetWindow[] = [];
                  document?.querySelectorAll('iframe')?.forEach((item) => item.contentWindow && res.push(item.contentWindow));
                  return res;
              })
            : new ClientConnector({
                  getTargetWindow: () => (window.parent ? [window.parent] : []),
                  module: connectorType,
              })
    ) as ConnectorType<T>;
})('framework');
