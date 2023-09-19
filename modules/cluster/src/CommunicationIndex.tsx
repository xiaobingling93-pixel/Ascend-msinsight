/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect } from 'react';
import { createRoot } from 'react-dom/client';
import { RootStoreContext, useRootStore } from './context/context';
import i18n from './i18n';
import './index.css';
import { store } from './store';
import connector from './connection';
import { observer } from 'mobx-react';
import { getSearchParams } from './utils/localUrl';
import { platform } from './platforms';
import { themeInstance, ThemeItem } from './theme/theme';
import CommunicationAnalysis from './components/communicationAnalysis/CommunicationAnalysis';
import { NOTIFICATION_HANDLERS } from './interface';

const Loading = (<div style={{ textAlign: 'center', top: '50%', position: 'absolute', width: '50px', left: 'calc(50% - 25px)' }}>
    <div className={'loading'} style={{ marginLeft: '15px' }}></div>
    <div>waiting</div>
</div>);

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
    return (<>{session !== undefined && session.clusterStatus === true ? <CommunicationAnalysis session={session} /> : Loading}</>);
});

window.dataSource = { remote: '127.0.0.1', port: 9000, dataPath: [] };
window.requestData = async (command, params, module) => {
    const data = await connector.fetch({ remote: window.dataSource, args: { command, params } },
        module !== undefined ? module : command?.split('/')[0]?.toLowerCase());
    return (data as any).body;
};

const root = createRoot(document.getElementById('root') as HTMLElement);
root.render(
    (<React.StrictMode>
        <RootStoreContext.Provider value={store}>
            <App />
        </RootStoreContext.Provider>
    </React.StrictMode>));

Object.entries(NOTIFICATION_HANDLERS).forEach(([ event, callback ]) => {
    connector.addListener(event, (e: MessageEvent<{ event: string; body: Record<string, unknown> }>) => {
        const res = e.data;
        if (res.body === undefined || typeof res.body !== 'object') {
            console.error('[notify]', 'Wrong notify format.');
            return;
        }
        callback(res.body);
    });
});
