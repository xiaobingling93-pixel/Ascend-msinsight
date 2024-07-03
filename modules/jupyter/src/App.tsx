/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect } from 'react';
import { ThemeProvider } from '@emotion/react';
import { useRootStore } from './context/context';
import connector from './connection';
import Jupyter from './components/jupyter/Jupyter';
import { themeInstance } from './theme/theme';

const app = observer(() => {
    const { sessionStore } = useRootStore();
    let session = sessionStore.activeSession;
    const getLanguage = (): void => {
        connector.send({
            event: 'getLanguage',
        });
    };
    useEffect(() => {
        session = sessionStore.activeSession;
        connector.send({ event: 'getParseStatus', body: { } });
        getLanguage();
    }, []);
    return <ThemeProvider theme={themeInstance.getThemeType()}>
        {session !== undefined ? <Jupyter session={session}/> : <></>}
    </ThemeProvider>;
});

export default app;
