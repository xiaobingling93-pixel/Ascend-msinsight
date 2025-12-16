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
