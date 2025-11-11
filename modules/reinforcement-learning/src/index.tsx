/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import { createRoot } from 'react-dom/client';
import '@insight/lib/i18n';
import '@insight/lib/style';
import { GlobalStyles } from '@insight/lib/theme';
import { disableShortcuts } from '@insight/lib/utils';
import { NOTIFICATION_HANDLERS } from '@/connection/handler';
import App from './App';
import { StoreProvider } from '@/stores';
import { LocaleProvider } from '@/components/common/LocaleProvider';
import { ThemeProvider } from '@/components/common/ThemeProvider';
import { registerMessageListeners } from '@/utils/messageDispatcher';

// 禁用右键刷新以及F5、Ctrl+R刷新
document.oncontextmenu = (): boolean => false;
disableShortcuts();

registerMessageListeners(NOTIFICATION_HANDLERS);

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    <React.StrictMode>
        <StoreProvider>
            <LocaleProvider>
                <ThemeProvider>
                    <GlobalStyles />
                    <App />
                </ThemeProvider>
            </LocaleProvider>
        </StoreProvider>
    </React.StrictMode>,
);
