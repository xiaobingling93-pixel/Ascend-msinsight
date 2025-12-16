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
import { App } from './App';
import { RootStoreContext } from './context/context';
import '@insight/lib/i18n';
import '@insight/lib/style';
import './theme.css';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';
import connector from './connection';
import { createRequest, type RequestOptions, errorCenter, WsError } from '@insight/lib';

interface CefQueryType {request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void};

declare global {
    interface Window {
        setTheme: (isDark: boolean) => void;
        request: (dataSource: DataSource, params: { command: string; params: Record<string, unknown> }, options?: RequestOptions) => Promise<any>;
        cefQuery: (obj: CefQueryType) => void;
        requestData: (method: string, params?: any, module?: string, options?: RequestOptions) => Promise<any>;
    }

    type LayerType = 'PROJECT' | 'CLUSTER' | 'HOST' | 'RANK' | 'COMPUTE' | 'IPYNB' | 'UNKNOWN';

    interface FileOrDirectory {
        type?: LayerType;
        name: string;
        path: string;
        children: FileOrDirectory[];
    }

    interface DataSource {
        remote: string;
        port: number;
        projectName: string;
        dataPath: string[];
        projectPath: string[];
        children: FileOrDirectory[];
        isBaseLine?: boolean;
        baseLineCardId?: string;
    }
};

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = (): boolean => false;
document.onkeydown = (event): boolean => event.key !== 'F5' && !(event.key === 'r' && event.ctrlKey);

window.request = async (dataSource, params, options): Promise<any> => {
    try {
        const data = await connector.fetch({ remote: dataSource, args: params });
        return (data as any).body;
    } catch (error: any) {
        const wsError = new WsError(error.code, error.message);
        if (!options?.silent) {
            errorCenter.handleError(wsError);
        }

        throw wsError;
    }
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

window.requestData = createRequest(connector);

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (<React.StrictMode>
        <RootStoreContext.Provider value={store}>
            <App />
        </RootStoreContext.Provider>
    </React.StrictMode>));
