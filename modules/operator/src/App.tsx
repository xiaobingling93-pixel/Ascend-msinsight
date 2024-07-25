/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { SharedConfigProvider } from 'lib/SharedConfigProvider';
import { useRootStore } from './context/context';
import Operator from './components/operator/Operator';
import { themeInstance } from './theme/theme';
import connector from './connection';
import { GlobalStyles } from 'lib/theme';

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
        getLanguage();
        connector.send({ event: 'getParseStatus', body: { from: 'Operator', request: 'operatorRankIds' } });
    }, []);

    const getLanguage = (): void => {
        connector.send({
            event: 'getLanguage',
        });
    };

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
