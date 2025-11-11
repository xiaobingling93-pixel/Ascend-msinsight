/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { ThemeProvider } from '@emotion/react';
import { useRootStore } from './context/context';
import Leaks from './components/leaks/leaks';
import { themeInstance } from './theme/theme';
import { registerEventHandlers, getInitStatus } from './index';
import { GlobalStyles } from '@insight/lib/theme';
import { SharedConfigProvider } from '@insight/lib';
import './index.css';
const App = observer(() => {
    const { sessionStore } = useRootStore();
    const [locale, setLocale] = useState<'zhCN' | 'enUS'>('zhCN');
    let session = sessionStore.activeSession;
    useEffect(() => {
        session = sessionStore.activeSession;
        registerEventHandlers();
        getInitStatus();
    }, []);
    useEffect(() => {
        if (session) {
            setLocale(session.language);
        }
    }, [session?.language]);
    return <ThemeProvider theme={themeInstance.getThemeType()}>
        <GlobalStyles />
        <SharedConfigProvider locale={locale}>
            {session !== undefined ? <Leaks session={session} /> : <></>}
        </SharedConfigProvider>

    </ThemeProvider>;
});

export default App;
