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

// 保留当前页面（时间范围、卡等）
export function savePageSetting(): void {
    const session = store.sessionStore.activeSession;
    if (session === undefined || session?.units.length === 0 || session?.projectName === undefined) {
        return;
    }
    const units = new UnitTreeTool().getSetting(session.units);
    const setting = {
        units,
        domainRange: session.domainRange,
    };
    session.pageSetting[session.projectName] = setting;
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

    recoverSetting(units: InsightUnit[], unitsSetting: InsightUnitSet[]): void {
        this.recoverSettingLimited(units, unitsSetting);
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

    private recoverSettingLimited(units: InsightUnit[], unitsSetting: InsightUnitSet[], iteration = 0): void {
        if (this.stopIteration(iteration)) {
            return;
        }
        units.forEach((unit, index) => {
            const unitset = unitsSetting[index];
            unit.isExpanded = unitset?.isExpanded ?? false;
            unit.onceExpand = unitset?.isExpanded ?? false;
            // 只有在自身状态是开启的情况下，才需要恢复子泳道的状态
            if (!(unitset?.isExpanded)) { return; }
            if (unit.children !== undefined && unit.children.length > 0 && Number(unitset.children.length) > 0) {
                this.recoverSettingLimited(unit.children, unitset.children, iteration + 1);
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
        const { domainRange, units } = setting;
        // 时间范围
        session.domainRange = domainRange;
        // 卡展开
        new UnitTreeTool().recoverSetting(session.units, units);
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
