/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { createRoot } from 'react-dom/client';
import { RootStoreContext } from './context/context';
import '@insight/lib/i18n';
import '@insight/lib/style';
import { store } from './store';
import App from './App';
import { sendShortcutKeys } from '@/connection/sendNotification';
import { disableShortcuts } from '@insight/lib/utils';

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (
        <RootStoreContext.Provider value={store}>
            <App/>
        </RootStoreContext.Provider>
    ));

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = (): boolean => false;
disableShortcuts([], [], (keyInfo: {hasCtrl: boolean;key: string}): void => {
    sendShortcutKeys(keyInfo);
});
