/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { createRoot } from 'react-dom/client';
import { App } from './App';
import { RootStoreContext } from './context/context';
import 'ascend-i18n';
import 'ascend-style';
import './theme.css';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';
import connector from './connection';
interface CefQueryType {request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void};

declare global {
    interface Window {
        // for inspecting some internal state
        debugInspector: {
            store?: unknown;
        };
        setTheme: (isDark: boolean) => void;
        request: (dataSource: DataSource, params: { command: string; params: Record<string, unknown> }) => Promise<any>;
        cefQuery: (obj: CefQueryType) => void;
        requestData: (method: string | RequestParams, params?: any, module?: string, voidResponse?: boolean) => Promise<any>;
    }

    interface DataSource {
        remote: string;
        port: number;
        projectName: string;
        dataPath: string[];
    }

    interface RequestParams {
        command: string;
        params: any;
        module?: string;
        voidResponse?: boolean;
        keepRawData?: boolean;
        bufferField?: string;
    }
};

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = (): boolean => false;
document.onkeydown = (event): boolean => event.key !== 'F5' && !(event.key === 'r' && event.ctrlKey);

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (<React.StrictMode>
        <RootStoreContext.Provider value={store}>
            <App />
        </RootStoreContext.Provider>
    </React.StrictMode>));

window.request = async (dataSource, params): Promise<any> => {
    const data = await connector.fetch({ remote: dataSource, args: params });
    return (data as any).body;
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

window.requestData = async (command, params, module, voidResponse = false): Promise<any> => {
    if (typeof command === 'object') {
        return await requestWithOptions(command);
    } else {
        return await requestWithMultiParams(command, params, module, voidResponse);
    }
};

async function requestWithMultiParams(command: string, params: any, module?: string, voidResponse = false): Promise<any> {
    const data = await connector.fetch({
        args: { command, params },
        module: module !== undefined ? module : String(command).split('/')[0]?.toLowerCase(),
        voidResponse,
    });
    return (data as any).body;
}

async function requestWithOptions({ command, params, module, voidResponse = false, keepRawData = false, bufferField }: RequestParams): Promise<any> {
    const data = await connector.fetch({
        args: { command, params },
        module: module !== undefined ? module : String(command).split('/')[0]?.toLowerCase(),
        voidResponse,
        keepRawData,
        bufferField,
    });
    return (data as any).body;
}
