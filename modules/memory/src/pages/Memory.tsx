/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { Session } from '../entity/session';
import { MemorySession, DataResourceType, GroupBy } from '../entity/memorySession';
import { useRootStore } from '../context/context';
import { Layout } from 'ascend-layout';
import MemoryHeader from '../components/MemoryHeader';
import MemoryLineChart from '../components/MemoryLineChart';
import MemoryDetailTable from '../components/MemoryDetailTable';
import { NormalDisplayStrategy, displayStrategyKey, displayStrategyMap, MemoryHeaderStrategy } from '../utils/strategyUtils';
import { memoryTypeGet, resourceTypeGet } from '../utils/RequestUtils';
import { customConsole as console } from 'ascend-utils';

const fetchMemoryType = (memorySession: MemorySession): void => {
    const memoryRankId = memorySession.rankIdCondition.value;
    if (memoryRankId === '') {
        return;
    }
    memoryTypeGet({ rankId: memoryRankId }).then((resp) => {
        const type = resp.type;
        const graphIdList = resp.graphId;
        runInAction(() => {
            memorySession.memoryType = type;
            if (graphIdList.length > 0) {
                memorySession.memoryGraphId = graphIdList[0];
            }
            memorySession.memoryGraphIdList = graphIdList;
        });
    }).catch(err => {
        console.error(err);
    });
};

const fetchResourceType = (memorySession: MemorySession): void => {
    const memoryRankId = memorySession.rankIdCondition.value;
    if (memoryRankId === '') {
        return;
    }
    resourceTypeGet({ rankId: memoryRankId }).then((resp) => {
        const type = resp.type;
        runInAction(() => {
            if (type === DataResourceType.MINDSPORE) {
                memorySession.groupId = GroupBy.DEFAULT;
            }
            memorySession.resourceType = type;
        });
    }).catch(err => {
        console.error(err);
    });
};

const getDisplayStrategy = (session: Session, memorySession: MemorySession): MemoryHeaderStrategy => {
    const resourceType = memorySession.resourceType === DataResourceType.PYTORCH
        ? displayStrategyKey.resourceType.pytorch
        : displayStrategyKey.resourceType.mindspore;
    const isHost = memorySession.hostCondition.options.length > 0
        ? displayStrategyKey.hasHostOptions.isHost
        : displayStrategyKey.hasHostOptions.notHost;
    const isCompard = session.compareRank.isCompare as boolean
        ? displayStrategyKey.isComparing.isCompared
        : displayStrategyKey.isComparing.notCompared;
    return displayStrategyMap[resourceType][isHost][isCompard];
};

const Memory = observer(({ session, isDark }: { session: Session; isDark: boolean }) => {
    const { memoryStore } = useRootStore();
    const memorySession = memoryStore.activeSession;
    const [strategy, setStrategy] = useState<MemoryHeaderStrategy>(new NormalDisplayStrategy());
    useEffect(() => {
        if (!memorySession) {
            return;
        };
        fetchMemoryType(memorySession);
        fetchResourceType(memorySession);
    }, [memorySession?.rankIdCondition.value]);

    useEffect(() => {
        if (!memorySession) {
            return;
        };
        setStrategy(getDisplayStrategy(session, memorySession));
    }, [memorySession?.hostCondition.options.length, memorySession?.resourceType, session.compareRank.isCompare]);

    return (
        memorySession
            ? <Layout>
                <MemoryHeader strategy={strategy} session={session} memorySession={memorySession}></MemoryHeader>
                <MemoryLineChart session={session} memorySession={memorySession} isDark={isDark}></MemoryLineChart>
                <MemoryDetailTable session={session} memorySession={memorySession}></MemoryDetailTable>
            </Layout>
            : <></>
    );
});

export default Memory;
