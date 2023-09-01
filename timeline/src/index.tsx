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
        request: (remote: string, params: { command: string; params: Record<string, unknown> }) => Promise<any>;
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

window.addEventListener('contextmenu', e => {
    e.stopImmediatePropagation();
}, true);

window.addEventListener('message', (e: MessageEvent<{ event: string; remote: string; body: string }>) => {
    if (NOTIFICATION_HANDLERS[e.data.event] === undefined ||
        NOTIFICATION_HANDLERS[e.data.body] === undefined ||
        NOTIFICATION_HANDLERS[e.data.remote] === undefined) {
        console.error('[notify]', 'Wrong notify format.');
        return;
    }
    NOTIFICATION_HANDLERS[e.data.event]({ remote: e.data.remote, body: e.data.body });
});

window.request = (remote: string, args: { command: string; params: Record<string, unknown> }) => {
    const reqFunction = JSON.parse(localStorage.getItem('request') as string) as (remote: string, moduleName: string, args: { command: string; params: Record<string, unknown> }) => Promise<any>;
    return reqFunction(remote, 'timeline', args);
};
