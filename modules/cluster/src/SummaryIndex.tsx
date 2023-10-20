import React, { useEffect } from 'react';
import { createRoot } from 'react-dom/client';
import { ThemeProvider } from '@emotion/react';
import { RootStoreContext, useRootStore } from './context/context';
import i18n from './i18n';
import './index.css';
import { store } from './store';
import connector from './connection';
import { observer } from 'mobx-react';
import { getSearchParams } from './utils/localUrl';
import { platform } from './platforms';
import { themeInstance, ThemeItem } from './theme/theme';
import AnalysisSummary from './pages/AnalysisSummary';
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
    connector.send({
        event: 'getParseStatus',
        body: { },
    });
    return (<ThemeProvider theme={themeInstance.getThemeType()}>
        { session?.clusterCompleted ? <AnalysisSummary session={session} /> : Loading}
    </ThemeProvider>);
});

window.dataSource = { remote: '127.0.0.1', port: 9000, dataPath: [] };
window.requestData = async (command, params, module) => {
    const data = await connector.fetch({
        remote: window.dataSource,
        args: { command, params },
        module: module !== undefined ? module : command?.split('/')[0]?.toLowerCase(),
    });
    return (data as any).body;
};
window.sendToModule = (body): void => {
    connector.send({
        event: 'moduleMessage',
        body,
        to: 2,
    });
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
