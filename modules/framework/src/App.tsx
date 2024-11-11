/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import { GlobalStyles } from 'ascend-theme';
import { SharedConfigProvider } from 'ascend-shared-config-provider';
import { DragDirection, useDraggableContainer } from 'ascend-use-draggable-container';
import { useRootStore } from './context/context';
import { themeInstance } from './theme/theme';
import RemoteManager from './components/RemoteManager/Index';
import Main from './components/Main';
import { Session } from '@/entity/session';
import { connectRemote } from '@/centralServer/server';
import { LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import './index.css';
import { registerEventListeners } from '@/utils/InitUtils';

const init = async(session: Session): Promise<void> => {
    // 连接后端（启动后第一次）
    if (!session.isVscode) {
        await connectRemote({ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] });
    }
    registerEventListeners();
};

const LEFT_WIDTH = 300;
const App = observer(() => {
    const { sessionStore } = useRootStore();
    const session = sessionStore.activeSession;
    const [locale] = useState<'zhCN' | 'enUS'>('zhCN');
    const currentTheme = themeInstance.getCurrentTheme();

    const [view] = useDraggableContainer({
        draggableWH: LEFT_WIDTH,
        dragDirection: DragDirection.LEFT,
        open: true,
    });

    useEffect(() => {
        if (currentTheme === 'dark') {
            document.documentElement.classList.add('dark');
        } else {
            document.documentElement.classList.remove('dark');
        }
    }, [currentTheme]);

    useEffect(() => {
        if (!session) {
            return;
        }

        // 启动
        init(session);
    }, []);

    return session !== undefined
        ? <ThemeProvider theme={themeInstance.getThemeType()}>
            <GlobalStyles />
            <SharedConfigProvider locale={locale}>
                {
                    view({
                        mainContainer: <Main session={session} />,
                        draggableContainer: <RemoteManager session={session} />,
                        id: 'framework',
                        padding: 16,
                    })
                }
            </SharedConfigProvider>
        </ThemeProvider>
        : <></>;
});

export default App;
