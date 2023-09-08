import React from 'react';
import { createRoot } from 'react-dom/client';
import { App } from './App';
import { RootStoreContext } from './context/context';
import './i18n';
import './index.css';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';

type CefQueryType = {request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void};

declare global {
    interface Window {
        // for inspecting some internal state
        debugInspector: {
            store?: unknown;
        };
        setTheme: (isDark: boolean) => void;
        request: (dataSource: DataSource, params: { command: string; params: Record<string, unknown> }) => Promise<any>;
        _resolve: (value: unknown) => void;
        _reject: (value?: any) => void;
        cefQuery: (obj: CefQueryType) => void;
    }

    interface DataSource {
        remote: string;
        port: number;
        dataPath: string[];
    }
};

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (<React.StrictMode>
        <RootStoreContext.Provider value={store}>
            <App />
        </RootStoreContext.Provider>
    </React.StrictMode>));

window.addEventListener('contextmenu', e => {
    e.stopImmediatePropagation();
}, true);

const connector = (() => {
    const send = (args: Record<string, unknown>, reject?: Function): void => {
        if (!window?.top?.postMessage) {
            const errMsg = '[connection]: missed postMessage function, please check your iframe element';
            console.error(errMsg);
            reject?.(new Error(errMsg));
            return;
        }
        window.top.postMessage(JSON.stringify(args), '*');
    };

    const _msgSequence: Map<MsgSequence, Function> = new Map();
    const listeners: Map<Handler, ListenerCb> = new Map();
    window.onmessage = (e: MessageEvent) => {
        const res = { ...e };
        res.data = JSON.parse(e.data);
        const _resolve = _msgSequence.get(res.data.id);
        if (_resolve) {
            _resolve(res.data);
            _msgSequence.delete(res.data.id);
        } else {
            listeners.forEach(cb => cb(res));
        }
    };

    type MsgSequence = number;
    let id: MsgSequence = 0;
    const fetch = async (args: Record<string, unknown>): Promise<unknown> => {
        return new Promise((resolve, reject) => {
            args.id = id;
            send(args, reject);
            _msgSequence.set(id++, resolve);
        });
    };

    type Handler = number;
    type ListenerCb = (res: MessageEvent) => void;
    let _handler = 0;
    const addListener = (cb: ListenerCb): Handler => {
        listeners.set(_handler, cb);
        return _handler++;
    };

    const removeListener = (handler: Handler): void => {
        listeners.delete(handler);
    };

    return {
        fetch,
        send,
        addListener,
        removeListener,
    };
})();

window.request = async (dataSource, params) => {
    const data = await connector.fetch({ event: 'request', remote: dataSource, args: params });
    return (data as any).body;
};

connector.addListener((e: MessageEvent<{ event: string; dataSource: DataSource; body: Record<string, unknown> }>) => {
    const res = e.data;
    if (NOTIFICATION_HANDLERS[res.event] === undefined ||
        res.body === undefined ||
        res.dataSource === undefined) {
        console.error('[notify]', 'Wrong notify format.');
        return;
    }
    NOTIFICATION_HANDLERS[res.event]({ dataSource: res.dataSource, body: res.body });
});
