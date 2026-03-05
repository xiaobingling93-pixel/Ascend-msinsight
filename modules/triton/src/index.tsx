/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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
import App from './App';
import { RootStoreContext } from './context/context';
import '@insight/lib/style';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';
import connector from './connection';
import '@insight/lib/i18n';
import { createRequest, disableShortcuts } from '@insight/lib/utils';

interface CefQueryType { request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void }

declare global {
    interface Window {
        setTheme: (isDark: boolean) => void;
        request: (method: string, params: Record<string, unknown>, module?: string) => Promise<any>;
        _resolve: (value: unknown) => void;
        _reject: (value?: any) => void;
        cefQuery: (obj: CefQueryType) => void;
    }

    interface DataSource {
        remote: string;
        port: number;
        dataPath: string[];
    }
}

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = (): boolean => false;
disableShortcuts();

window.request = createRequest(connector);

export function registerEventHandlers(): void {
    Object.entries(NOTIFICATION_HANDLERS).forEach(([event, callback]) => {
        connector.addListener(event, (e: MessageEvent<{ event: string; body: Record<string, unknown> }>) => {
            const res = e.data;
            if (res.body === undefined || typeof res.body !== 'object') {
                return;
            }
            callback(res.body);
        });
    });
}

export function getInitStatus(): void {
    connector.send({
        event: 'getParseStatus',
        body: {
            from: 'Triton',
            requests: ['language', 'theme', 'tritonParsed'],
        },
    });
}

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (<React.StrictMode>
        <RootStoreContext.Provider value={store}>
            <App />
        </RootStoreContext.Provider>
    </React.StrictMode>));
