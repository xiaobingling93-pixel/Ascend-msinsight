/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState } from 'react';
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import { SharedConfigProvider } from 'ascend-shared-config-provider';
import { useRootStore } from './context/context';
import { themeInstance } from './theme/theme';
import { GlobalStyles } from 'ascend-theme';
import { DragDirection, useDraggableContainer } from 'ascend-use-draggable-container';
import RemoteManager from './components/RemoteManager/Index';
import Main from './components/Main';

export const LEFT_WIDTH = 300;
const App = observer(() => {
    const { sessionStore } = useRootStore();
    const session = sessionStore.activeSession;
    const [locale] = useState<'zhCN' | 'enUS'>('zhCN');

    const [view] = useDraggableContainer({
        draggableWH: LEFT_WIDTH,
        dragDirection: DragDirection.LEFT,
        open: true,
    });

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
