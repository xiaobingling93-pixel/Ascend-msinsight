/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect } from 'react';
import { createRoot } from 'react-dom/client';
import { ThemeProvider } from '@emotion/react';
import { RootStoreContext, useRootStore } from './context/context';
import i18n from './i18n';
import './Summary.css';
import { store } from './store';
import connector from './connection';
import { observer } from 'mobx-react';
import { getSearchParams } from './utils/localUrl';
import { themeInstance } from './theme/theme';
import AnalysisSummary from './pages/AnalysisSummary';
import { Loading } from './index';

export const App = observer(() => {
    const { sessionStore } = useRootStore();
    let session = sessionStore.activeSession;
    const lang = getSearchParams('language');
    useEffect(() => {
        session = sessionStore.activeSession;
        i18n.changeLanguage(lang === 'zh' ? 'zh' : 'en');
        themeInstance.setCurrentTheme('dark');
        window.setTheme(true);
        connector.send({ event: 'getParseStatus', body: { } });
    }, []);
    return (<ThemeProvider theme={themeInstance.getThemeType()}>
        {session !== undefined && <AnalysisSummary session={session} />}
        <div className={`fullmask ${session?.clusterCompleted ? 'hide' : ''}`}>{Loading}</div>
    </ThemeProvider>);
});

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (<React.StrictMode>
        <RootStoreContext.Provider value={store}>
            <App />
        </RootStoreContext.Provider>
    </React.StrictMode>));
