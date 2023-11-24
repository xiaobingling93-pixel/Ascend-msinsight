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

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = () => false;
document.onkeydown = (event) => event.key !== 'F5' && !(event.key === 'r' && event.ctrlKey);
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
window.requestData = async (command, params, module) => {
    const data = await connector.fetch({
        args: { command, params },
        module: module !== undefined ? module : command?.split('/')[0]?.toLowerCase(),
    });
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
