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
