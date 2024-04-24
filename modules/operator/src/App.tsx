/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { ThemeProvider } from '@emotion/react';
import { observer } from 'mobx-react';
import React, { useEffect } from 'react';
import { useRootStore } from './context/context';
import Operator from './components/operator/Operator';
import { themeInstance } from './theme/theme';

const App = observer(() => {
    const { sessionStore } = useRootStore();
    let session = sessionStore.activeSession;
    useEffect(() => {
        session = sessionStore.activeSession;
        window.setTheme(true);
    }, []);
    return session !== undefined
        ? <ThemeProvider theme={themeInstance.getThemeType()}>
            <Operator session={session} />
        </ThemeProvider>
        : <></>;
});

export default App;
