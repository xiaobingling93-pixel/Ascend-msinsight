/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useState } from 'react';
import { createRoot } from 'react-dom/client';
import { RootStoreContext } from './context/context';
import './i18n';
import './index.css';
import { store } from './store';
import { NOTIFICATION_HANDLERS } from './interface';
import connector from './connection';
import { Button, Input } from 'antd';
const { TextArea } = Input;

type CefQueryType = {request: string; onSuccess: (response: string) => void; onFailure: (errorCode: number, errorMessage: string) => void};

declare global {
    interface Window {
        // for inspecting some internal state
        debugInspector: {
            store?: unknown;
        };
        setTheme: (isDark: boolean) => void;
        request: (dataSource: DataSource, params: { command: string; params: Record<string, unknown> }) => Promise<any>;
        _resolve: (value: unknown) => void;
        _reject: (value?: any) => void;
        cefQuery: (obj: CefQueryType) => void;
    }

    interface DataSource {
        remote: string;
        port: number;
        dataPath: string[];
    }
};

function getInitData(): any {
    const initData = { dataPath: 'D:\\data\\0818_analyzed', module: 'communication', command: 'communication/duration/operatorNames', rucan: '{"rankId":"0","iterationId":"0","rankList":["1"],"stage":"(0, 1, 2, 3, 4, 5, 6, 7)"}' };

    let x = sessionStorage.getItem('dataPath');
    if (x !== null && x !== '') {
        initData.dataPath = x;
    }
    x = sessionStorage.getItem('module');
    if (x !== null && x !== '') {
        initData.module = x;
    }
    x = sessionStorage.getItem('command');
    if (x !== null && x !== '') {
        initData.command = x;
    }
    x = sessionStorage.getItem('rucan');
    if (x !== null && x !== '') {
        initData.rucan = x;
    }
    return initData;
}
const App = (): JSX.Element => {
    const initData = getInitData();
    const [ data, setData ] = useState('');
    const [ param, setParam ] = useState(initData);
    const sendTo = async(): Promise<any> => {
        setData('');
        const { module, command, rucan } = param;
        // eslint-disable-next-line no-eval
        const res = await window.requestData(command, eval(`( ${rucan} )`), module);
        if (res !== undefined) {
            const text = JSON.stringify(res, null, 2).slice(0, 1000);
            setData(text);
        }
    };
    return <div style={{ padding: '20px', background: 'white' }}>
        <div>module：</div>
        <Input onChange={(e) => { setParam({ ...param, module: e.target.value.trim() }); sessionStorage.setItem('module', e.target.value.trim()); }} value={param.module}/>
        <div>接口：</div>
        <Input onChange={(e) => { setParam({ ...param, command: e.target.value.trim() }); sessionStorage.setItem('command', e.target.value.trim()); } } value={param.command}/>
        <div>入参：</div>
        <TextArea rows={4} onChange={(e) => { setParam({ ...param, rucan: e.target.value.trim() }); sessionStorage.setItem('rucan', e.target.value.trim()); }} value={param.rucan}/>
        <Button type="primary" onClick={sendTo}>Send</Button>
        <TextArea rows={30} value={data} onChange={(e) => { setData(e.target.value); }}/>
    </div>;
};

window.dataSource = { remote: '127.0.0.1', port: 9000, dataPath: [] };
window.requestData = async (command, params, module) => {
    const data = await connector.fetch({ remote: window.dataSource, args: { command, params } }, module);
    return (data as any).body;
};

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (
        <RootStoreContext.Provider value={store}>
            <div>
                <App />
            </div>

        </RootStoreContext.Provider>
    ));

window.addEventListener('contextmenu', e => {
    e.stopImmediatePropagation();
}, true);

window.request = async (dataSource, params) => {
    const data = await connector.fetch({ remote: dataSource, args: params });
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

declare global {
    interface Window {
        requestData: (method: string, params: any, module?: string) => Promise<any>;
        dataSource: DataSource;
    }
};
