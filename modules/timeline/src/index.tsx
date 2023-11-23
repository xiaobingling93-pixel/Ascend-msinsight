import React from 'react';
import { createRoot } from 'react-dom/client';
import { App } from './App';
import { RootStoreContext } from './context/context';
import './i18n';
import './index.css';
import './theme.css';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';
import connector from './connection';

type CefQueryType = {request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void};

declare global {
    interface Window {
        // for inspecting some internal state
        debugInspector: {
            store?: unknown;
        };
        setTheme: (isDark: boolean) => void;
        request: (dataSource: DataSource, params: { command: string; params: Record<string, unknown> }) => Promise<any>;
        cefQuery: (obj: CefQueryType) => void;
        requestData: (method: string, params: any, module?: string) => Promise<any>;
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

window.request = async (dataSource, params) => {
    const data = await connector.fetch({ remote: dataSource, args: params });
    return (data as any).body;
};

Object.entries(NOTIFICATION_HANDLERS).forEach(([ event, callback ]) => {
    connector.addListener(event, (e: MessageEvent<{ event: string; body: Record<string, unknown> }>) => {
        const res = e.data;
        if (res.body === undefined || typeof res.body !== 'object') {
            console.error('[notify]', 'Wrong notify format.');
            return;
        }
        callback(res.body);
    });
});

window.requestData = async (command, params, module) => {
    const data = await connector.fetch({
        args: { command, params },
        module: module !== undefined ? module : command?.split('/')[0]?.toLowerCase(),
    });
    return (data as any).body;
};
