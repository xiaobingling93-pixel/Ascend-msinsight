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

import { register } from './register';
import type { Session } from '../entity/session';
import { runInAction } from 'mobx';
import type { ChartDesc, InsightUnit } from '../entity/insight';
import type { StackStatusConfig } from '../entity/chart';
import { getUnitUniqueId } from '../utils';

export function updateThreadsToFetch(session: Session, isExpand: boolean, unitOrFetchMap: InsightUnit | Map<string, InsightUnit>): void {
    if (unitOrFetchMap instanceof Map) {
        if (isExpand) {
            for (const [key, value] of unitOrFetchMap) {
                session.threadsToFetch.set(key, value);
            }
        } else {
            for (const key of unitOrFetchMap.keys()) {
                session.threadsToFetch.delete(key);
            }
        }
    } else {
        const unitKey = getUnitUniqueId(unitOrFetchMap);
        const isExisted = session.threadsToFetch.has(unitKey);
        if (isExpand) {
            !isExisted && session.threadsToFetch.set(unitKey, unitOrFetchMap);
        } else {
            isExisted && session.threadsToFetch.delete(unitKey);
        }
    }
}

const expandUnits = (_unit: InsightUnit, shouldExpand: boolean, session: Session, _threadsToFetch: Map<string, InsightUnit>): void => {
    const subUnits = _unit.children ?? [];
    if (!subUnits.length) return;
    subUnits.forEach(unit => {
        expandUnits(unit, shouldExpand, session, _threadsToFetch);
        unit.isExpanded = shouldExpand;
        const isThread = unit.name === 'Thread';
        if (isThread && !unit.hasExpanded) {
            const unitKey = getUnitUniqueId(unit);
            _threadsToFetch.set(unitKey, unit);
        }
        if (isThread && unit.collapsible) {
            const chart = unit.chart as ChartDesc<'stackStatus'>;
            (chart.config as StackStatusConfig).isCollapse = shouldExpand;
            unit.collapseAction?.(unit);
        }
    });
};

const collapseOrExpandAll = (session: Session, shouldExpand: boolean): void => {
    // 必须只选中一个才能调用“收起、展开全部子项”菜单项
    if (session.selectedUnits.length !== 1) {
        return;
    }
    const selectedUnit = session.selectedUnits[0];
    runInAction(() => {
        const _threadsToFetchMap: Map<string, InsightUnit> = new Map();
        if (selectedUnit !== undefined) {
            expandUnits(selectedUnit, shouldExpand, session, _threadsToFetchMap);
            selectedUnit.isExpanded = true;
        }
        session.renderTrigger = !session.renderTrigger;
        if (_threadsToFetchMap.size > 0) {
            updateThreadsToFetch(session, shouldExpand, _threadsToFetchMap);
        }
    });
};

const haveExpandedChildren = (_unit: InsightUnit): boolean => {
    if (!_unit.collapsible || !_unit.children || _unit.children.length === 0) {
        return false;
    }

    return _unit.children.some(child => (child?.collapsible && child.isExpanded) ?? haveExpandedChildren(child));
};

const haveCollapsedChildren = (_unit: InsightUnit): boolean => {
    if (!_unit.collapsible || !_unit.children || _unit.children.length === 0) {
        return false;
    }

    return _unit.children.some(child => (child.collapsible !== undefined && child.collapsible && !child.isExpanded) || haveCollapsedChildren(child));
};

const isCollapseAllVisible = (session: Session): boolean => {
    // 必须只选中一个才能显示“收起全部子项”菜单项
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit !== undefined) {
        return haveExpandedChildren(selectedUnit);
    }
    return false;
};

const isExpandAllVisible = (session: Session): boolean => {
    // 必须只选中一个才能显示“展开全部子项”菜单项
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit !== undefined) {
        const isCollapsed = selectedUnit.collapsible && !selectedUnit.isExpanded;
        const haveChildUnits = selectedUnit.children && selectedUnit.children.length > 0;
        if (isCollapsed && haveChildUnits) {
            return true;
        }
        return haveCollapsedChildren(selectedUnit);
    }
    return false;
};

export const actionExpandAllUnits = register({
    name: 'expandAllUnits',
    label: 'timeline:contextMenu.Expand all',
    visible: (session) => isExpandAllVisible(session),
    perform: (session): void => {
        collapseOrExpandAll(session, true);
    },
});

export const actionCollapseAllUnits = register({
    name: 'collapseAllUnits',
    label: 'timeline:contextMenu.Collapse all',
    visible: (session) => isCollapseAllVisible(session),
    perform: (session): void => {
        collapseOrExpandAll(session, false);
    },
});
