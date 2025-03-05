/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
import { createRoot } from 'react-dom/client';
import { RootStoreContext } from './context/context';
import 'ascend-i18n';
import 'ascend-style';
import './index.css';
import App from './App';
import connector from './connection';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';
import { disableShortcuts } from 'ascend-utils';
import { shortcutSwitchFindWindow } from './connection/handler';
import type { KeydownInfo } from '@/utils/interface';

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = (): boolean => false;
disableShortcuts([], [], (keyInfo: KeydownInfo): void => {
    shortcutSwitchFindWindow(keyInfo);
});

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
        requestData: <T extends object>(method: string, params: object, module?: string) => Promise<T>;
    }
};
window.requestData = async <T extends object>(command: string, params: object, module?: string): Promise<T> => {
    const data = await connector.fetch({
        args: { command, params },
        module: module !== undefined ? module : String(command).split('/')[0]?.toLowerCase(),
    });
    return (data as {body: T})?.body;
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
