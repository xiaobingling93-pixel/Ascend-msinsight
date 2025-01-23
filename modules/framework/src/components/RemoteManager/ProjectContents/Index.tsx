/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect } from 'react';
import { observer } from 'mobx-react';
import { store } from '@/store';
import connector from '@/connection';
import ContextMenu from './ContextMenu';
import Contents from './Contents';

const Index = observer(() => {
    const session = store.sessionStore.activeSession;

    useEffect(() => {
        const { projectName, dataPath } = session.activeDataSource;
        let rankId: string = '';
        // 比对数据
        if (session.isCompareStatus) {
            rankId = session.compareSet.comparison.rankId;
        } else {
            // 切换目录
            if (projectName !== '' && dataPath.length > 0) {
                rankId = session.getRankId({ projectName, filePath: dataPath[0] });
            }
        }
        // 通知页签
        connector.send({
            event: 'switchDirectory',
            body: { rankId, isCompare: session.isCompareStatus },
        });
    }, [session.activeDataSource.dataPath, session.isCompareStatus, session.compareSet.comparison.rankId]);

    return <>
        <Contents session={session}/>
        <ContextMenu session={session}/>
    </>;
});

export default Index;
