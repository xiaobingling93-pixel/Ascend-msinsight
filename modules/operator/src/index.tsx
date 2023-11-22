/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React from 'react';
import { createRoot } from 'react-dom/client';
import { RootStoreContext } from './context/context';
import './i18n';
import './index.css';
import { store } from './store';
import App from './App';
import { NOTIFICATION_HANDLERS } from './interface';
import connector from './connection';

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (
        <RootStoreContext.Provider value={store}>
            <App/>
        </RootStoreContext.Provider>
    ));

type CefQueryType = {request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void};

declare global {
    interface Window {
        hljs: Function;
        setTheme: Function;
        requestData: (method: string, params: any, module?: string) => Promise<any>;
        dataSource: DataSource;
        cefQuery: (obj: CefQueryType) => void;
    }
    interface DataSource {
        remote: string;
        port: number;
        dataPath: string[];
    }
};
window.dataSource = { remote: '127.0.0.1', port: 9000, dataPath: [] };
window.requestData = async (command, params, module) => {
    const data = await connector.fetch({
        remote: window.dataSource,
        args: { command, params },
        module: module !== undefined ? module : command?.split('/')[0]?.toLowerCase(),
    });
    return (data as any).body;
};
window.setTheme = (isDark: boolean) => {
    document.body.className = isDark ? 'theme_dark' : 'theme_light';
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
