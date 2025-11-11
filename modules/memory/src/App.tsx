/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { SharedConfigProvider } from '@insight/lib';
import { useRootStore } from './context/context';
import { themeInstance } from './theme/theme';
import Memory from './pages/Memory';
import { GlobalStyles } from '@insight/lib/theme';
import { getInitStatus, registerEventHandlers } from './bootstrap';

export const App = observer(() => {
    const { sessionStore } = useRootStore();
    const session = sessionStore.activeSession;
    const [locale, setLocale] = useState<'zhCN' | 'enUS'>('zhCN');
    useEffect(() => {
        if (session) {
            setLocale(session.language);
        }
    }, [session?.language]);

    useEffect(() => {
        registerEventHandlers();
        getInitStatus();
    }, []);

    return (
        <ThemeProvider theme={themeInstance.getThemeType()}>
            <GlobalStyles />
            <SharedConfigProvider locale={locale}>
                {session !== undefined ? <Memory session={session} isDark={themeInstance.getCurrentTheme() === 'dark'} /> : <></>}
            </SharedConfigProvider>
        </ThemeProvider>
    );
});
