/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect } from 'react';
import { useRootStore } from './context/context';
import connector from './connection';
import HotMethod from './components/hotMethod/HotMethod';
import Detail from './components/detail/Index';

const app = observer(({ page }: {page?: string}) => {
    const { sessionStore } = useRootStore();
    const session = sessionStore.activeSession;
    useEffect(() => {
        connector.send({ event: 'getParseStatus', body: { } });
    }, []);
    let dom;
    if (session === undefined) {
        dom = <></>;
    } else if (page === 'detail') {
        dom = <Detail session={session}/>;
    } else {
        dom = <HotMethod session={session} />;
    }
    return dom;
});

export default app;
