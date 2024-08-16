/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
import { createRoot } from 'react-dom/client';
import { RootStoreContext } from './context/context';
import 'ascend-style';
import App from './App';
import connector from './connection';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';
import { log } from './components/Common';

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (
        <RootStoreContext.Provider value={store}>
            <App/>
        </RootStoreContext.Provider>
    ));

interface CefQueryType {request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void};

declare global {
    interface Window {
        setTheme: (isDark: boolean) => void;
        cefQuery: (obj: CefQueryType) => void;
        requestData: (method: string, params: object, module?: string) => Promise<object>;
    }
};

window.requestData = async (command, params, module): Promise<object> => {
    const data = await connector.fetch({
        args: { command, params },
        module: module !== undefined ? module : String(command).split('/')[0]?.toLowerCase(),
    });
    return (data as {body: object}).body;
};

Object.entries(NOTIFICATION_HANDLERS).forEach(([event, callback]) => {
    connector.addListener(event, (e: MessageEvent<{ event: string; body: Record<string, unknown> }>) => {
        const res = e.data;
        if (res.body === undefined || typeof res.body !== 'object') {
            log('[notify]', 'Wrong notify format.');
            return;
        }
        callback(res.body);
    });
});
