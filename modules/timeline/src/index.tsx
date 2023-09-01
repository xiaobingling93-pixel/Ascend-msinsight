import React from 'react';
import { createRoot } from 'react-dom/client';
import { App } from './App';
import { RootStoreContext } from './context/context';
import './i18n';
import './index.css';
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

window.addEventListener('contextmenu', e => {
    e.stopImmediatePropagation();
}, true);

window.addEventListener('message', (...args) => {
    console.log('frame window is', window, args);
});

console.log('frame window is', window);
