/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
import { createRoot } from 'react-dom/client';
import { RootStoreContext } from './context/context';
import './i18n';
import 'lib/style/index';
import './index.css';
import App from './App';
import connector from './connection';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';

export function init(page?: string): void {
    const root = createRoot(document.getElementById('root') as HTMLElement);
    root.render(
        (
            <RootStoreContext.Provider value={store}>
                <App page={page}/>
            </RootStoreContext.Provider>
        ));
}

declare global {
    interface Window {
        setTheme: (isDark: boolean) => void;
        requestData: (method: string, params: object, module?: string) => Promise<object>;
    }
};
window.requestData = async (command, params, module): Promise<object> => {
    const data = await connector.fetch({
        args: { command, params },
        module: module !== undefined ? module : String(command).split('/')[0]?.toLowerCase(),
    });
    return (data as {body: object})?.body;
};
window.setTheme = (isDark: boolean): void => {
    document.body.className = isDark ? 'theme_dark' : 'theme_light';
};

Object.entries(NOTIFICATION_HANDLERS).forEach(([event, callback]) => {
    connector.addListener(event, (e: MessageEvent<{ event: string; body: Record<string, unknown> }>) => {
        const res = e.data;
        if (res.body === undefined || typeof res.body !== 'object') {
            return;
        }
        callback(res.body);
    });
});
