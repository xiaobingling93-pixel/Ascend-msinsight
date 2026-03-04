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
import type { InsightUnit } from '../entity/insight';
import { store } from '../store';
import { switchPinned } from '../components/ChartContainer/unitPin';
import { Session } from '../entity/session';
import { updateThreadsToFetch } from '../actions/actionExpandUnits';

interface RecoverSettingLimitedItem {
    units: InsightUnit[];
    unitsSetting: InsightUnitSet[];
    pinnedUnits: InsightUnit[];
    session: Session;
    iteration?: number;
}

// 保留当前页面（时间范围、卡等）
export function savePageSetting(): void {
    const session = store.sessionStore.activeSession;
    if (session === undefined || session?.units.length === 0 || session?.projectName === undefined) {
        return;
    }
    const units = new UnitTreeTool().getSetting(session.units);
    session.pageSetting[session.projectName] = {
        units,
        domainRange: session.domainRange,
        pinnedUnits: session.pinnedUnits,
    };
}

export interface InsightUnitSet {
    isExpanded: boolean;
    children: InsightUnitSet[];
}

class UnitTreeTool {
    private readonly _maxIteration = 1000000;

    getSetting(units: InsightUnit[]): InsightUnitSet[] {
        return this.getSettingLimited(units);
    }

    recoverSetting(units: InsightUnit[], unitsSetting: InsightUnitSet[], pinnedUnits: InsightUnit[] = [], session: Session): void {
        this.recoverSettingLimited({ units, unitsSetting, pinnedUnits, session });
    }

    private stopIteration(index: number): boolean {
        return index > this._maxIteration;
    }

    private getSettingLimited(units: InsightUnit[], iteration = 0): InsightUnitSet[] {
        if (this.stopIteration(iteration)) {
            return [];
        }
        return units.map(unit => ({
            isExpanded: unit.isExpanded,
            children: unit.children ? this.getSettingLimited(unit.children ?? [], iteration + 1) : [],
        }));
    }

    private recoverSettingLimited({ units, unitsSetting, pinnedUnits = [], session, iteration = 0 }: RecoverSettingLimitedItem): void {
        if (this.stopIteration(iteration)) {
            return;
        }
        units.forEach((unit, index) => {
            const settingUnit = unitsSetting[index];
            unit.isExpanded = settingUnit?.isExpanded ?? false;
            unit.onceExpand = settingUnit?.isExpanded ?? false;
            if (unit.name === 'Thread' && unit.isExpanded) {
                updateThreadsToFetch(session, unit.isExpanded, unit);
            }

            // 校验次unit在上一次是否被置顶
            const pinnedUnitIdx = pinnedUnits.findIndex(item => {
                const { cardId, processId, threadId, label } = item.metadata || {};
                const metadata = unit.metadata;
                // cardId、processId、threadId 在threadId为空时，不能判断泳道的唯一性，临时增加label判断
                return metadata.cardId === cardId && metadata.processId === processId && metadata.threadId === threadId && label === metadata.label;
            });
            // 若上一次被置顶，则恢复置顶状态
            if (pinnedUnitIdx !== -1) {
                switchPinned(unit);
                pinnedUnits[pinnedUnitIdx] = unit;
            }
            // 泳道未展开不进行子泳道的展开恢复，但需要恢复子泳道的置顶状态
            if (unit.children?.length && settingUnit?.children?.length) {
                this.recoverSettingLimited({ units: unit.children, unitsSetting: settingUnit.children, pinnedUnits, session, iteration: iteration + 1 });
            }
        });
    }
}

export function recoverPageSetting(): void {
    const session = store.sessionStore.activeSession;
    if (session === undefined) {
        return;
    }
    const { pageSetting, projectName } = session;
    if (projectName !== undefined && pageSetting[projectName] !== undefined) {
        const setting = session.pageSetting[projectName];
        if (!setting) {
            return;
        }
        const { domainRange, units, pinnedUnits } = setting;
        // 时间范围
        session.domainRange = domainRange;
        session.pinnedUnits = [...pinnedUnits];
        // 卡展开
        new UnitTreeTool().recoverSetting(session.units, units, session.pinnedUnits, session);
    }
}

interface SingleDataPath {
    singleDataPath: string;
    dataSource: DataSource;
}
export function updatePageSetting({ type, data }: {type: string; data?: any}): void {
    const session = store.sessionStore.activeSession;
    if (session === undefined) {
        return;
    }
    switch (type) {
        case 'updateProjectName':
            {
                const { oldProjectName, newProjectName } = (data ?? {}) as Record<string, string>;
                if (session.pageSetting[oldProjectName] !== undefined) {
                    session.pageSetting[newProjectName] = session.pageSetting[oldProjectName];
                    session.pageSetting[oldProjectName] = undefined;
                }
            }
            break;
        case 'removeSingleDataPath':
            {
                const { singleDataPath, dataSource } = (data ?? {}) as SingleDataPath;
                const index = dataSource?.dataPath?.findIndex(path => path === singleDataPath);
                if (index >= 0 && session.pageSetting[dataSource.projectName] !== undefined) {
                    session.pageSetting[dataSource.projectName]?.units?.splice(index, 1);
                }
            }
            break;
        case 'removeDataSource':
            {
                const { projectName } = (data ?? {}) as DataSource;
                if (session.pageSetting[projectName] !== undefined) {
                    session.pageSetting[projectName] = undefined;
                }
            }
            break;
        case 'reset':
            session.pageSetting = {};
            break;
        default:
            break;
    }
}
