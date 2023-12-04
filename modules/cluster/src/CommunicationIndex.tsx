/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect } from 'react';
import { createRoot } from 'react-dom/client';
import { ThemeProvider } from '@emotion/react';
import { RootStoreContext, useRootStore } from './context/context';
import i18n from './i18n';
import { store } from './store';
import connector from './connection';
import { observer } from 'mobx-react';
import { getSearchParams } from './utils/localUrl';
import { platform } from './platforms';
import { themeInstance, ThemeItem } from './theme/theme';
import CommunicationAnalysis from './components/communicationAnalysis/CommunicationAnalysis';
import { Loading } from './index';

export const App = observer(() => {
    const { insightStore, sessionStore } = useRootStore();
    let session = sessionStore.activeSession;
    const lang = getSearchParams('language');
    useEffect(() => {
        insightStore.loadTemplates().then(() => {
            session = sessionStore.activeSession;
        });
        i18n.changeLanguage(lang === 'zh' ? 'zh' : 'en');
        platform.initTheme().then((res: ThemeItem) => {
            themeInstance.setCurrentTheme(res);
            window.setTheme(res === 'dark');
        });
    }, []);
    connector.send({ event: 'getParseStatus', body: { } });
    return (<ThemeProvider theme={themeInstance.getThemeType()}>
        {session?.clusterCompleted ? <CommunicationAnalysis session={session} /> : Loading}
    </ThemeProvider>);
});

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (<React.StrictMode>
        <RootStoreContext.Provider value={store}>
            <App />
        </RootStoreContext.Provider>
    </React.StrictMode>));
