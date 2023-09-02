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

window.addEventListener('message', (e: MessageEvent<{ event: string; dataSource: DataSource; body: Record<string, unknown> }>) => {
    console.log(e);
    if (NOTIFICATION_HANDLERS[e.data.event] === undefined ||
        e.data.body === undefined ||
        e.data.dataSource === undefined) {
        console.error('[notify]', 'Wrong notify format.');
        return;
    }
    NOTIFICATION_HANDLERS[e.data.event]({ dataSource: e.data.dataSource, body: e.data.body });
});
