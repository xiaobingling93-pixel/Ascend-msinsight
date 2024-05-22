/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect } from 'react';
import { useRootStore } from './context/context';
import connector from './connection';
import Jupyter from './components/jupyter/Jupyter';

const app = observer(() => {
    const { sessionStore } = useRootStore();
    let session = sessionStore.activeSession;
    useEffect(() => {
        session = sessionStore.activeSession;
        connector.send({ event: 'getParseStatus', body: { } });
    }, []);
    return session !== undefined ? <Jupyter session={session} /> : <></>;
});

export default app;
