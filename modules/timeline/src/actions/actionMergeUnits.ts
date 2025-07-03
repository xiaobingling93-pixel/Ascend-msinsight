/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { runInAction } from 'mobx';
import { register } from './register';
import type { Session } from '../entity/session';
import { ThreadMetaData } from '../entity/data';
import { ThreadUnit } from '../insight/units/AscendUnit';
import { type ChartDesc, InsightUnit, UnitHeight } from '../entity/insight';
import { message } from 'antd';
import type { StackStatusConfig } from '../entity/chart';
import i18n from 'ascend-i18n';

const clearSelectedUnits = (session: Session): void => {
    session.selectedUnits = [];
    session.selectedUnitKeys = [];
};

const findInsertIndex = (allUnits: InsightUnit[], selectedUnits: InsightUnit[]): number => {
    let minIndex = Infinity;

    for (const sUnit of selectedUnits) {
        const sMetaData = sUnit.metadata as ThreadMetaData;

        const indexInAllUnits = allUnits.findIndex(aUnit => {
            const aMetaData = aUnit.metadata as ThreadMetaData;

            return aMetaData.threadId === sMetaData.threadId;
        });
        if (indexInAllUnits !== -1 && indexInAllUnits < minIndex) {
            minIndex = indexInAllUnits;
        }
    }

    return minIndex === Infinity ? 0 : minIndex;
};

const isStreamUnit = (unit: InsightUnit): boolean => {
    const metaData = unit.metadata as ThreadMetaData;

    if (!metaData.processName || !metaData.threadName) {
        return false;
    }

    return metaData.processName.startsWith('Ascend Hardware') && metaData.threadName.startsWith('Stream');
};

const extractThreadIds = (units: InsightUnit[]): string[] => {
    return units.flatMap(unit => {
        const { threadIdList, threadId } = unit.metadata as ThreadMetaData;
        return threadIdList ?? [threadId];
    }) as string[];
};

const getThreadNameList = (threadIds: string[], firstUnit: InsightUnit): string[] => {
    const childrenUnits = firstUnit.parent?.children ?? [];
    const threadMap = new Map<string, ThreadMetaData>();

    for (const unit of childrenUnits) {
        const meta = unit.metadata as ThreadMetaData;
        if (meta.threadId) {
            threadMap.set(meta.threadId, meta);
        }
    }

    return threadIds.sort().map(threadId => {
        const meta = threadMap.get(threadId);
        return meta?.threadName?.replace(/^Stream\s*/, '') ?? '';
    });
};

const getMergedUnitMetaData = (selectedUnits: InsightUnit[]): ThreadMetaData => {
    const [firstUnit] = selectedUnits;
    const firstMeta = firstUnit.metadata as ThreadMetaData;

    const threadIdList = extractThreadIds(selectedUnits);
    const threadNameList = getThreadNameList(threadIdList, firstUnit);

    return {
        ...firstMeta,
        threadIdList,
        threadId: '',
        threadName: `Stream Merged (${threadNameList.join(', ')})`,
    };
};

// 标记被合并的泳道
const markMergedUnits = (selectedUnits: InsightUnit[], allUnitList: InsightUnit[]): void => {
    for (const unit of selectedUnits) {
        const meta = unit.metadata as ThreadMetaData;
        if (meta.threadIdList) {
            const index = allUnitList.indexOf(unit);
            if (index === -1) { continue; }
            allUnitList.splice(index, 1);
        } else {
            unit.isMerged = true;
        }
    }
};

const createMergedUnit = (mergedMeta: ThreadMetaData): InsightUnit => {
    const threadUnit = new ThreadUnit(mergedMeta);
    const chart = threadUnit.chart as ChartDesc<'stackStatus'>;
    const chartConfig = chart.config as StackStatusConfig;

    chart.height = UnitHeight.STANDARD;
    chart.renderTooltip = (data): Map<string, string> => new Map([
        ['Name', data.name],
        ['Stream', data.threadId ?? '-'],
    ]);
    chartConfig.maxDepth = 0;
    chartConfig.isCollapse = false;
    threadUnit.collapsible = false;
    threadUnit.notifications = [(): string => i18n.t('Merged unit', { ns: 'timeline' })];

    return threadUnit;
};

const mergeUnits = (session: Session): void => {
    const selectedUnits = session.selectedUnits;
    if (selectedUnits.length === 0) { return; }

    const [firstUnit] = selectedUnits;
    const { cardId } = firstUnit.metadata as ThreadMetaData;

    // 1. 校验
    const allStreamUnits = selectedUnits.every(isStreamUnit);
    if (!allStreamUnits) {
        message.warning(i18n.t('timeline:MergeStreamOnly'));
        return;
    }

    const allSameCard = selectedUnits.every(unit => {
        const metaData = unit.metadata as ThreadMetaData;
        return metaData.cardId === cardId;
    });
    if (!allSameCard) {
        message.warning(i18n.t('timeline:MergeInSameCardOnly'));
        return;
    }

    const unitParent = firstUnit.parent;
    if (!unitParent?.children) { return; }

    // 2. 构建合并泳道的 metadata
    const mergedMeta = getMergedUnitMetaData(selectedUnits);
    if (!mergedMeta) { return; }

    // 3. 创建 ThreadUnit 类型的合并泳道
    const threadUnit = createMergedUnit(mergedMeta);

    // 4. 插入合并泳道 & 更新原始泳道状态
    const insertIndex = findInsertIndex(unitParent.children, selectedUnits);

    runInAction(() => {
        if (!unitParent?.children) { return; }
        unitParent.children.splice(insertIndex, 0, threadUnit);

        markMergedUnits(selectedUnits, unitParent.children);
        clearSelectedUnits(session);
        session.renderTrigger = !session.renderTrigger;
    });
};

const hasMergedUnit = (selectedUnits: InsightUnit[]): boolean => {
    const mergedUnits = selectedUnits.filter(unit => {
        const metaData = unit.metadata as ThreadMetaData;

        return metaData.threadIdList;
    });

    return mergedUnits.length > 0;
};

// 从 parent 中移除指定 unit
const removeUnitFromParent = (unit: InsightUnit, parent?: { children?: InsightUnit[] }): void => {
    const children = parent?.children;
    if (!children) { return; }

    const index = children.indexOf(unit);
    if (index !== -1) {
        children.splice(index, 1);
    }
};

// 获取合并泳道的 threadId 并 移除合并泳道
const getMergedThreadIdsAndRemoveMergedUnit = (selectedUnits: InsightUnit[], parent: InsightUnit): Set<string> => {
    const threadIds = new Set<string>();

    for (const unit of selectedUnits) {
        const metadata = unit.metadata as ThreadMetaData;

        if (Array.isArray(metadata.threadIdList)) {
            removeUnitFromParent(unit, parent);
            metadata.threadIdList.forEach(id => threadIds.add(id));
        }
    }

    return threadIds;
};

// 取消指定泳道的合并标记
const unmarkMergedUnits = (threadIds: Set<string>, children?: InsightUnit[]): void => {
    if (!children) { return; }

    for (const unit of children) {
        const metadata = unit.metadata as ThreadMetaData;
        if (threadIds.has(metadata.threadId as string)) {
            unit.isMerged = false;
        }
    }
};

const unmergeUnits = (session: Session): void => {
    const selectedUnits = session.selectedUnits;

    if (!hasMergedUnit(selectedUnits)) {
        return;
    }

    runInAction(() => {
        const parent = selectedUnits[0].parent;
        const children = parent?.children;

        if (!children) { return; }

        const mergedThreadIds = getMergedThreadIdsAndRemoveMergedUnit(selectedUnits, parent);

        unmarkMergedUnits(mergedThreadIds, children);
        clearSelectedUnits(session);
        session.renderTrigger = !session.renderTrigger;
    });
};

export const actionMergeUnits = register({
    name: 'mergeUnits',
    label: 'timeline:contextMenu.Merge Units',
    visible: (session) => {
        return session.selectedUnits.length > 1;
    },
    perform: (session): void => {
        mergeUnits(session);
    },
});

export const actionUnmergeUnits = register({
    name: 'unmergeUnits',
    label: 'timeline:contextMenu.Unmerge Units',
    visible: (session) => {
        return hasMergedUnit(session.selectedUnits);
    },
    perform: (session): void => {
        unmergeUnits(session);
    },
});
