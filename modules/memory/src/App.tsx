/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { SharedConfigProvider } from 'ascend-shared-config-provider';
import { useRootStore } from './context/context';
import { themeInstance } from './theme/theme';
import Memory from './pages/Memory';
import connector from './connection';
import { GlobalStyles } from 'ascend-theme';
import { registerEventHandlers } from './bootstrap';

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
        getLanguage();
        connector.send({ event: 'getParseStatus', body: { from: 'Memory', request: 'memoryRankIds' } });
        connector.send({ event: 'getTheme' });
    }, []);

    const getLanguage = (): void => {
        connector.send({
            event: 'getLanguage',
        });
    };

    return (
        <ThemeProvider theme={themeInstance.getThemeType()}>
            <GlobalStyles />
            <SharedConfigProvider locale={locale}>
                {session !== undefined ? <Memory session={session} isDark={themeInstance.getCurrentTheme() === 'dark'} /> : <></>}
            </SharedConfigProvider>
        </ThemeProvider>
    );
});
