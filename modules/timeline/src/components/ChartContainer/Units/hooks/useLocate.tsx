import { autorun, runInAction } from 'mobx';
import React from 'react';
import { PreOrderFlattenOptions, TreeNode } from '../../../../entity/common';
import { InsightUnit, UnitMatcher } from '../../../../entity/insight';
import { Session } from '../../../../entity/session';
import { getAutoKey } from '../../../../utils/dataAutoKey';
/**
 * Searches a list of given @param units recursively in pre-order, comparing them with @param matcher, and save the result path in @path
 *
 * @param units the root units to start searching from
 * @param matcher matcher
 * @param path if a matching unit is found, keeps the path from root to the matching unit.
 * @returns true if found a matching unit
 */
const searchUnit = (units: InsightUnit[], matcher: UnitMatcher['target'], path: InsightUnit[]): boolean => {
    for (const unit of units) {
        path.push(unit);
        if (matcher(unit)) {
            return true;
        }
        if (unit.children && searchUnit(unit.children, matcher, path)) {
            return true;
        }
        path.pop();
    }
    return false;
};

const getTargetUnit = (units: InsightUnit[], matcher: UnitMatcher['target']): InsightUnit | undefined => {
    const path: InsightUnit[] = [];
    if (!searchUnit(units, matcher, path)) {
        return;
    }
    runInAction(() => {
        // expand all parent units
        for (const unit of path) {
            if (unit.collapsible && !unit.isExpanded) {
                unit.collapseAction?.(unit);
            }
            unit.isExpanded = true;
        }
    });
    return path[path.length - 1];
};

const getNormalUnitHeight = (unitsArea: InsightUnit[], orderOptions: OrderOptions, targetUnit: InsightUnit): number | undefined => {
    const flattenUnits = orderOptions.preOrderFlatten(unitsArea, 0, orderOptions.options);
    let findResult = false;
    let height = 0;
    for (const unit of flattenUnits) {
        if (unit === targetUnit) {
            findResult = true;
            break;
        }
        height += unit.height() + 1;
    }
    height -= 10;
    return findResult ? height : undefined;
};

export type OrderOptions = {
    preOrderFlatten: <T>(tree: Array<TreeNode<T>>, currentLevel: number, options?: PreOrderFlattenOptions<T> | undefined) => T[];
    options: PreOrderFlattenOptions<InsightUnit>;
};

export const useJumpTarget = (session: Session, unitsArea: InsightUnit[], supportJump: boolean,
    orderOptions: OrderOptions, dom: HTMLDivElement | null): void => {
    React.useEffect(() => autorun(
        () => {
            if (dom === null || !supportJump) { return; }
            if (session.locateUnit === undefined) { return; }
            const targetUnit = getTargetUnit(session.units, session.locateUnit.target);
            if (targetUnit !== undefined) {
                session.selectedUnitKeys = [getAutoKey(targetUnit)];
                session.selectedUnits = [targetUnit];
                session.locateUnit?.onSuccess(targetUnit);
                const scrollHResult = getNormalUnitHeight(unitsArea, orderOptions, targetUnit);
                if (scrollHResult !== undefined) {
                    dom?.scrollTo(0, scrollHResult);
                }
            }
            runInAction(() => {
                session.locateUnit = undefined;
            });
        },
    ), [session, dom, unitsArea]);
};
