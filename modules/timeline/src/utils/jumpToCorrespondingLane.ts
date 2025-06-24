/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { runInAction } from 'mobx';
import type { UnitDetail } from '../api/interface';
import { store } from '../store';
import { InsightUnit } from '../entity/insight';

/**
 * 选中特定泳道
 * @param {UnitDetail} unitDetail 泳道详情信息
 */
const jumpToCorrespondingLane = (unitDetail: UnitDetail): void => {
    const {
        cardId: cid,
        pid,
        propsCardId,
    } = unitDetail;
    const session = store.sessionStore.activeSession;
    if (session === undefined) {
        return;
    }

    runInAction(() => {
        session.locateUnit = {
            target: (unit: InsightUnit): boolean => {
                const { cardId, processId } = unit.metadata;
                return (cid === cardId || propsCardId === cardId) && processId === pid;
            },
            onSuccess: (): void => {},
            showDetail: false,
        };
    });
};

export default jumpToCorrespondingLane;
