type TargetWindow = Window | null | undefined;
type ReservedEventHandler = 'request';
type EventHanlder = string;
type SendParams<T extends EventHanlder> = {
    event: T extends ReservedEventHandler ? never : T;
    [x: string]: unknown;
};
type ListenerCallback = (res: MessageEvent) => void;
abstract class BaseConnector {
    protected readonly _errMsgType = 'postMessage';
    protected invalidFunc: null | (() => void) = null;
    protected _targetWindow: TargetWindow;
    protected _getTargetWindow: () => TargetWindow;
    protected _listeners: Map<EventHanlder, ListenerCallback> = new Map();
    protected _broadcastListeners: ListenerCallback[] = [];

    constructor(getTargetWindow: () => TargetWindow) {
        if (typeof window !== 'object') {
            const errMsg = 'cannot find global Window object, please check your runtime environment';
            console.error(this.printErrMsg(errMsg));
            this.invalidFunc = () => { throw new Error(this.printErrMsg(errMsg)); };
        }
        this._getTargetWindow = getTargetWindow;

        window.onmessage = (event: MessageEvent) => {
            const res = { ...event };
            res.data = JSON.parse(event.data);
            const listener = this._listeners.get(res.data.event);
            if (res.data.event === 'request') {
                this.awaitFetch(res);
            } else if (res.data.event === 'broadcast') {
                this._broadcastListeners.forEach(callback => callback(res));
            } else if (listener) {
                listener(res);
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

    send<T extends EventHanlder>(params: SendParams<T>, reject?: Function): void {
        if (this.invalidFunc) {
            this.invalidFunc();
            return;
        }
        if (!this._targetWindow) {
            this._targetWindow = this._getTargetWindow();
        }
        if (!this._targetWindow?.postMessage) {
            const errMsg = 'missed postMessage function, please check your iframe element';
            console.warn(this.printErrMsg(errMsg));
            reject?.(new Error(errMsg));
            return;
        }
        this._targetWindow.postMessage(JSON.stringify(params), '*');
    }

    addListener<T extends EventHanlder>(event: T extends ReservedEventHandler ? never : T, callback: ListenerCallback): T {
        if (this.invalidFunc) {
            this.invalidFunc();
        }
        if (this._listeners.has(event)) {
            console.warn(this.printErrMsg('detected some even type was overrided, this maybe cause some mistake, pleas check it'));
        }
        this._listeners.set(event, callback);
        return event;
    }

    removeListener<T extends EventHanlder>(event: T extends ReservedEventHandler ? never : T): void {
        if (this.invalidFunc) {
            this.invalidFunc();
        }
        this._listeners.delete(event);
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

class ClientConnector extends BaseConnector {
    private readonly _msgSequence: Map<FetchSequenceID, Function> = new Map();
    private _curFetchSequenceID: FetchSequenceID = 0;

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

    async fetch<T extends EventHanlder>(params: Record<string, unknown>): Promise<unknown> {
        return new Promise((resolve, reject) => {
            params.id = this._curFetchSequenceID;
            params.event = 'request';
            this.send(params as SendParams<T>, reject);
            this._msgSequence.set(this._curFetchSequenceID++, resolve);
        });
    };
};

type TypeForConnector = {
    server: ServerConnector;
    client: ClientConnector;
};

type ConnectorType = keyof TypeForConnector;
export default (function connectorFactory<T extends ConnectorType>(connectorType: T): TypeForConnector[T] {
    return (connectorType === 'server' ? new ServerConnector(() => document?.querySelector('iframe')?.contentWindow) : new ClientConnector(() => window?.top)) as TypeForConnector[T];
})('server');
