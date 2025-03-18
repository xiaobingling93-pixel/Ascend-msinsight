/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { type InsightUnit } from '../entity/insight';
import { forEach } from 'lodash';
import { runInAction } from 'mobx';
import { colorPalette, getTimeOffset } from '../insight/units/utils';
import type { ThreadMetaData } from '../entity/data';
import { calculateDomainRange } from '../components/CategorySearch';
import { hashToNumber } from './colorUtils';
import { ThreadUnit } from '../insight/units/AscendUnit';
import type { OpDetail } from '../api/interface';
import { store } from '../store';

export const getOperatingSystem = function (): string {
    const userAgent = navigator.userAgent.toLowerCase();

    if (userAgent.includes('windows')) {
        return 'Windows';
    } else if (userAgent.includes('macintosh') || userAgent.includes('mac os')) {
        return 'Mac OS';
    } else if (userAgent.includes('linux')) {
        return 'Linux';
    } else {
        return 'Unknown';
    }
};

export const getRootUnit = (units: InsightUnit[]): InsightUnit[] => {
    const result: InsightUnit[] = [];
    forEach(units, (unit) => {
        if (unit.parent) {
            if (!result.includes(unit.parent)) {
                result.push(unit.parent);
            }
        } else {
            result.push(unit);
        }
    });
    return result;
};

/**
 * 在泳道中选中特定算子
 * @param {OpDetail} opDetail 算子详情信息
 */
export const jumpToUnitOperator = (opDetail: OpDetail): void => {
    const { id, cardId, tid, pid, depth, duration, name, timestamp } = opDetail;
    const session = store.sessionStore.activeSession;
    if (session === undefined) {
        return;
    }

    runInAction(() => {
        session.locateUnit = {
            target: (unit: InsightUnit): boolean => {
                return unit instanceof ThreadUnit &&
                    unit.metadata.cardId === cardId &&
                    unit.metadata.threadId === tid &&
                    unit.metadata.processId === pid;
            },
            onSuccess: (unit): void => {
                const startTime = timestamp - getTimeOffset(session, unit.metadata as ThreadMetaData);
                const [rangeStart, rangeEnd] = calculateDomainRange(session, startTime, duration);
                session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                session.selectedData = {
                    id,
                    startTime,
                    name,
                    color: colorPalette[hashToNumber(name, colorPalette.length)],
                    duration,
                    depth,
                    threadId: tid,
                    processId: pid,
                    cardId,
                    startRecordTime: session.startRecordTime,
                    showSelectedData: true,
                };
            },
            showDetail: false,
        };
    });
};
