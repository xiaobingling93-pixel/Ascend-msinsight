type TargetWindow = Window;
type ReservedEventHandler = 'request';
type EventHanlder = string;
type SendParams<T extends EventHanlder> = {
    event: T extends ReservedEventHandler ? never : T;
    [x: string]: unknown;
};
type ListenerCallback = (res: MessageEvent) => void;
type ListenerHandler = { event: EventHanlder; sequence: number };
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
            this.invalidFunc = () => { throw new Error(this.printErrMsg(errMsg)); };
        }
        this._getTargetWindows = getTargetWindow;

        window.onmessage = (event: MessageEvent) => {
            const res = { ...event };
            res.data = JSON.parse(event.data);
            const listener = this._listeners.get(res.data.event);
            if (res.data.event === 'request') {
                this.awaitFetch(res);
            } else if (listener) {
                listener.forEach(cb => cb?.(res));
            } else {
                console.warn(this.printErrMsg('missed [event] in your message, please check your params, or maybe have an invalid send'));
            }
        };
        Object.defineProperty(window, 'onmessage', {
            configurable: false,
            set: () => {
                console.warn(this.printErrMsg('the property \'window.onmessage\' in used, reassign it is invalid'));
            },
        });
    }

    protected printErrMsg(errMsg: string): string {
        return `[${this._errMsgType}]: ${errMsg}`;
    }

    protected abstract awaitFetch(e: MessageEvent): void;

    send<T extends EventHanlder>(params: SendParams<T>, to?: SendTargetKey, reject?: Function): void {
        if (this.invalidFunc) {
            this.invalidFunc();
            return;
        }
        this._targetWindows = this._getTargetWindows();

        if ((to !== undefined && this._targetWindows[to]?.postMessage !== undefined) || this._targetWindows.length === 0) {
            const errMsg = 'missed postMessage function, please check your iframe element';
            console.warn(this.printErrMsg(errMsg));
            reject?.(new Error(errMsg));
            return;
        }
        this._targetWindows.forEach(targetWindow => { targetWindow.postMessage(JSON.stringify(params), '*'); });
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
            listeners.filter(item => item).length === 0 && this._listeners.delete(event);
        }
    }
};

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
        this.send({ event: 'request', id: event.data.id, ...res } as SendParams<EventHanlder>);
    }
};

type FetchParams = {
    event?: never;
    [x: string]: unknown;
};
class ClientConnector extends BaseConnector {
    private readonly _msgSequence: Map<FetchSequenceID, Function> = new Map();
    private _curFetchSequenceID: FetchSequenceID = 0;
    private readonly _module: TypeForConnector;

    constructor({
        getTargetWindow,
        module,
    }: {
        getTargetWindow: GetTragetWindows;
        module: TypeForConnector;
    }) {
        super(getTargetWindow);
        this._module = module;
    }

    protected awaitFetch(res: MessageEvent): void {
        const _resolve = this._msgSequence.get(res.data.id);
        if (!_resolve) {
            console.warn(this.printErrMsg(
                'cannot find relative resolve for this fetch, this maybe cause omission of message, please check your connection',
            ));
            return;
        }
        _resolve(res.data);
        this._msgSequence.delete(res.data.id);
    }

    send<T extends EventHanlder>(params: SendParams<T>, to?: SendTargetKey, reject?: Function, module?: TypeForConnector): void {
        params.module = module ?? this._module;
        super.send(params, undefined, reject);
    }

    async fetch(params: FetchParams, module?: TypeForConnector): Promise<unknown> {
        return new Promise((resolve, reject) => {
            params.id = this._curFetchSequenceID;
            (params as Record<string, unknown>).event = 'request';
            this.send(params as any, undefined, reject, module);
            this._msgSequence.set(this._curFetchSequenceID++, resolve);
        });
    };
};

type TypeForConnector = 'framework' | string;
type ConnectorType<T extends TypeForConnector> = T extends 'framework' ? ServerConnector : ClientConnector;
export default (function connectorFactory<T extends TypeForConnector>(connectorType: T): ConnectorType<T> {
    return (connectorType === 'framework'
        ? new ServerConnector(() => {
            const res: TargetWindow[] = [];
            document?.querySelectorAll('iframe')?.forEach(item => item.contentWindow && res.push(item.contentWindow));
            return res;
        })
        : new ClientConnector({
            getTargetWindow: () => window.top ? [window.top] : [],
            module: connectorType,
        })) as ConnectorType<T>;
})('timeline');
