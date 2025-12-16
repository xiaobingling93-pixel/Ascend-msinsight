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

const expandUnits = (_unit: InsightUnit, shouldExpand: boolean): void => {
    if (_unit.children && _unit.children?.length > 0) {
        _unit?.children?.forEach(childUnit => {
            expandUnits(childUnit, shouldExpand);
            childUnit.isExpanded = shouldExpand;
            if (childUnit.name === 'Thread' && childUnit.collapsible) {
                const chart = childUnit.chart as ChartDesc<'stackStatus'>;
                (chart.config as StackStatusConfig).isCollapse = shouldExpand;
                childUnit.collapseAction?.(childUnit);
            }
        });
    }
};

const collapseOrExpandAll = (session: Session, shouldExpand: boolean): void => {
    // 必须只选中一个才能调用“收起、展开全部子项”菜单项
    if (session.selectedUnits.length !== 1) {
        return;
    }
    const selectedUnit = session.selectedUnits[0];
    runInAction(() => {
        if (selectedUnit !== undefined) {
            expandUnits(selectedUnit, shouldExpand);
            selectedUnit.isExpanded = true;
        }
        session.renderTrigger = !session.renderTrigger;
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
