/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { SharedConfigProvider } from 'ascend-shared-config-provider';
import { useRootStore } from './context/context';
import Operator from './components/operator/Operator';
import { themeInstance } from './theme/theme';
import connector from './connection';
import { GlobalStyles } from 'ascend-theme';

const App = observer(() => {
    const { sessionStore } = useRootStore();
    let session = sessionStore.activeSession;
    const [locale, setLocale] = useState<'zhCN' | 'enUS'>('zhCN');

    useEffect(() => {
        if (session) {
            setLocale(session.language);
        }
    }, [session?.language]);

    useEffect(() => {
        session = sessionStore.activeSession;
        window.setTheme(true);
        connector.send({ event: 'getParseStatus', body: { from: 'Operator', requests: ['language', 'theme', 'operatorRankIds', 'directory'] } });
    }, []);

    return session !== undefined
        ? <ThemeProvider theme={themeInstance.getThemeType()}>
            <GlobalStyles />
            <SharedConfigProvider locale={locale}>
                <Operator session={session} />
            </SharedConfigProvider>
        </ThemeProvider>
        : <></>;
});

export default App;
