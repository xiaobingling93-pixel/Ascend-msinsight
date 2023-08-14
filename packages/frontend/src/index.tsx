import React from 'react';
import { createRoot } from 'react-dom/client';
import { App } from './App';
import { InsightConnection } from './connection/client';
import { RootStoreContext } from './context/context';
import './i18n';
import './index.css';
import { NOTIFICATION_HANDLERS } from './interface';
import { store } from './store';

type CefQueryType = {request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void};

declare global {
    interface Window {
        // for inspecting some internal state
        debugInspector: {
            store?: unknown;
        };
        setTheme: (isDark: boolean) => void;
        request: (method: string, params: Record<string, unknown>) => Promise<any>;
        cefQuery: (obj: CefQueryType) => void;
    }
};

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (<React.StrictMode>
        <RootStoreContext.Provider value={store}>
            <App />
        </RootStoreContext.Provider>
    </React.StrictMode>));

(async () => {
    const conn = new InsightConnection(NOTIFICATION_HANDLERS);
    await conn.reset();
    await conn.connect();
    window.request = conn.fetch.bind(conn);

    window.request('unit/chart', { param: 'some data' })
        .then(result => {
            console.log('request 1 finished', result);
        }).catch(e => {
            console.warn('OK: request 1 failed', e);
        });
    window.request('unit/chart', { chartId: 'some data' })
        .then(result => {
            console.log('request 2 finished', result);
        }).catch(e => {
            console.warn('OK: request 2 failed', e);
        });
    window.request('unit/chart', { chartId: 1, otherParam: '123' })
        .then(result => {
            console.log('OK: request 3 finished', result);
        }).catch(e => {
            console.warn('request 3 failed', e);
        });
    window.request('test', { innerData: 1 })
        .then(result => {
            console.log('OK: request 4 finished', result);
        }).catch(e => {
            console.warn('request 4 failed', e);
        });
})();
