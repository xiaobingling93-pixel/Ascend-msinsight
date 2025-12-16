/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import React from 'react';
import { createRoot } from 'react-dom/client';
import { RootStoreContext } from './context/context';
import '@insight/lib/i18n';
import '@insight/lib/style';
import './index.css';
import App from './App';
import connector from './connection';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';
import { disableShortcuts } from '@insight/lib/utils';
import { shortcutSwitchFindWindow } from './connection/handler';
import type { KeydownInfo } from '@/utils/interface';

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = (): boolean => false;
disableShortcuts([], [], (keyInfo: KeydownInfo): void => {
    shortcutSwitchFindWindow(keyInfo);
});

declare global {
    interface Window {
        setTheme: (isDark: boolean) => void;
        requestData: <T extends object>(method: string, params: Record<string, unknown>, module?: string) => Promise<T | undefined> ;
    }
};
const REQUEST_ERROR_TIMEOUT = 'page request error timeout';
window.requestData = async <T extends object>(command: string, config: Record<string, unknown>, module?: string): Promise<T | undefined> => {
    const { retry, ...params } = config;
    try {
        const data = await withTimeout(connector.fetch({
            args: { command, params },
            module: module !== undefined ? module : String(command).split('/')[0]?.toLowerCase(),
        }), retry ? 1000 : 0, REQUEST_ERROR_TIMEOUT);
        return (data as {body: T})?.body;
    } catch (error) {
        if ((error as any)?.message === REQUEST_ERROR_TIMEOUT) {
            return window.requestData(command, params, module);
        }
        return undefined;
    };
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

function withTimeout(promise: any, timeout: number, message?: string): Promise<any> {
    if (timeout <= 0) {
        return promise;
    }
    return Promise.race([
        promise, // 原始的异步操作
        new Promise((resolve, reject) =>
            setTimeout(() => reject(new Error(message ?? 'withTimeout')), timeout),
        ), // 超时逻辑
    ]);
}

export function init(page?: string): void {
    const root = createRoot(document.getElementById('root') as HTMLElement);
    root.render(
        (
            <RootStoreContext.Provider value={store}>
                <App page={page}/>
            </RootStoreContext.Provider>
        ));
}
