/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect } from 'react';
import { observer } from 'mobx-react';
import { store } from '@/store';
import ContextMenu from './ContextMenu';
import Contents from './Contents';
import { sendDirectory } from '@/connection/sendNotification';

const Index = observer(() => {
    const session = store.sessionStore.activeSession;

    useEffect(() => {
        sendDirectory();
    }, [session.activeDataSource.dataPath, session.isCompareStatus, session.compareSet.comparison.rankId]);

    return <>
        <Contents session={session}/>
        <ContextMenu session={session}/>
    </>;
});

export default Index;
