/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { runInAction } from 'mobx';
import { SharedConfigProvider } from '@insight/lib';
import { createRoot } from 'react-dom/client';
import { ThemeProvider } from '@emotion/react';
import { RootStoreContext, useRootStore } from './context/context';
import { store } from './store';
import connector from './connection';
import { observer } from 'mobx-react';
import { themeInstance, GlobalStyles } from '@insight/lib/theme';
import AnalysisSummary from './pages/AnalysisSummary';
import { Loading } from './index';

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

    /* 添加 selectedClusterPath 属性事件监听，保证切换 selectedClusterPath 时页面能整体更新 */
    useEffect(() => {
        if (!session) { return; }
        runInAction(() => {
            session.resetForClusterChange();
            session.renderId = ++session.renderId % 1000;
        });
    }, [session?.selectedClusterPath]);

    return (<ThemeProvider theme={themeInstance.getThemeType()}>
        <GlobalStyles />
        <SharedConfigProvider locale={locale}>
            {session?.clusterCompleted && <AnalysisSummary session={session} />}
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
