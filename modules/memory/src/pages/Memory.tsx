/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { Session } from '../entity/session';
import { MemorySession, DataResourceType, GroupBy } from '../entity/memorySession';
import { useRootStore } from '../context/context';
import { Layout } from '@insight/lib/components';
import MemoryHeader from '../components/MemoryHeader';
import MemoryLineChart from '../components/MemoryLineChart';
import MemoryDetailTable from '../components/MemoryDetailTable';
import { NormalDisplayStrategy, displayStrategyKey, displayStrategyMap, MemoryHeaderStrategy } from '../utils/strategyUtils';
import { memoryTypeGet, resourceTypeGet } from '../utils/RequestUtils';
import { customConsole as console } from '@insight/lib/utils';

const fetchMemoryType = (memorySession: MemorySession): void => {
    const memoryCard = memorySession.getSelectedRankValue();
    if (memoryCard.rankInfo.rankId === '') {
        return;
    }
    memoryTypeGet({ rankId: memoryCard.rankInfo.rankId, dbPath: memoryCard.dbPath }).then((resp) => {
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
    const memoryCard = memorySession.getSelectedRankValue();
    if (memoryCard.rankInfo.rankId === '') {
        return;
    }
    resourceTypeGet({ rankId: memoryCard.rankInfo.rankId, dbPath: memoryCard.dbPath }).then((resp) => {
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
    }, [memorySession?.selectedRankId]);

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
