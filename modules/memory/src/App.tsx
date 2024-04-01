/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import React, { useEffect, useState, useRef } from 'react';
import { useRootStore } from './context/context';
import { themeInstance } from './theme/theme';
import MemoryAnalysis from './pages/MemoryAnalysis';
import connector from './connection';
import { getSearchParams } from './utils/localUrl';
import i18n from './i18n';

export const App = observer(() => {
    const { sessionStore } = useRootStore();
    const [themeDark, setThemeDark] = useState<boolean>(true);
    const hasListenerRef = useRef<boolean>(false);
    let session = sessionStore.activeSession;
    const lang = getSearchParams('language');
    useEffect(() => {
        session = sessionStore.activeSession;
        i18n.changeLanguage(lang === 'zh' ? 'zh' : 'en');
        themeInstance.setCurrentTheme('dark');
        window.setTheme(true);
    }, []);

    !hasListenerRef.current && connector.addListener('setTheme', (e) => {
        hasListenerRef.current = true;
        setThemeDark(e.data.body.isDark);
    });

    return (
        <ThemeProvider theme={themeInstance.getThemeType()}>
            {session !== undefined ? <MemoryAnalysis session={session} isDark={themeDark} /> : <></>}
        </ThemeProvider>
    );
});
