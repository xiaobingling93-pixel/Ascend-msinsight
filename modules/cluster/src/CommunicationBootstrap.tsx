/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { SharedConfigProvider } from 'ascend-shared-config-provider';
import { createRoot } from 'react-dom/client';
import { ThemeProvider } from '@emotion/react';
import { RootStoreContext, useRootStore } from './context/context';
import { store } from './store';
import connector from './connection';
import { observer } from 'mobx-react';
import { themeInstance } from './theme/theme';
import CommunicationAnalysis from './components/communication/CommunicationAnalysis';
import { Loading } from './index';
import { GlobalStyles } from 'ascend-theme';

export const App = observer(() => {
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
        getLanguage();
        themeInstance.setCurrentTheme('dark');
        window.setTheme(true);
        connector.send({ event: 'getParseStatus', body: { } });
    }, []);

    const getLanguage = (): void => {
        connector.send({
            event: 'getLanguage',
        });
    };

    return (<ThemeProvider theme={themeInstance.getThemeType()}>
        <GlobalStyles />
        <SharedConfigProvider locale={locale}>
            {session !== undefined && <CommunicationAnalysis session={session} />}
            <div className={`fullmask ${session?.clusterCompleted ? 'hide' : ''}`}>{Loading}</div>
        </SharedConfigProvider>
    </ThemeProvider>);
});

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (<React.StrictMode>
        <RootStoreContext.Provider value={store}>
            <App />
        </RootStoreContext.Provider>
    </React.StrictMode>));
