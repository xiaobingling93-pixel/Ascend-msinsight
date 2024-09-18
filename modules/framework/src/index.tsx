/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { createRoot } from 'react-dom/client';
import { RootStoreContext } from './context/context';
import 'ascend-i18n';
import 'ascend-style';
import { store } from './store';
import App from './App';

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (
        <RootStoreContext.Provider value={store}>
            <App/>
        </RootStoreContext.Provider>
    ));
