/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import React, { useEffect, useState, useRef } from 'react';
import { SharedConfigProvider } from 'lib/SharedConfigProvider';
import { useRootStore } from './context/context';
import { themeInstance } from './theme/theme';
import MemoryAnalysis from './pages/MemoryAnalysis';
import connector from './connection';
import { GlobalStyles } from 'lib/theme';

export const App = observer(() => {
    const { sessionStore } = useRootStore();
    const [themeDark, setThemeDark] = useState<boolean>(true);
    const hasListenerRef = useRef<boolean>(false);
    let session = sessionStore.activeSession;
    const [locale, setLocale] = useState<'zhCN' | 'enUS'>('zhCN');

    useEffect(() => {
        if (session) {
            setLocale(session.language);
        }
    }, [session?.language]);

    useEffect(() => {
        session = sessionStore.activeSession;
        getLanguage();
        themeInstance.setCurrentTheme('dark');
        window.setTheme(true);
        connector.send({ event: 'getParseStatus', body: { from: 'Memory', request: 'memoryRankIds' } });
    }, []);

    if (!hasListenerRef.current) {
        connector.addListener('setTheme', (e: any) => {
            hasListenerRef.current = true;
            setThemeDark(e.data.body.isDark);
        });
    };

    const getLanguage = (): void => {
        connector.send({
            event: 'getLanguage',
        });
    };

    return (
        <ThemeProvider theme={themeInstance.getThemeType()}>
            <GlobalStyles />
            <SharedConfigProvider locale={locale}>
                {session !== undefined ? <MemoryAnalysis session={session} isDark={themeDark} /> : <></>}
            </SharedConfigProvider>
        </ThemeProvider>
    );
});
