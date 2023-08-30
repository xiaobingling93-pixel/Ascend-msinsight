import { autorun, runInAction } from 'mobx';
import React from 'react';
import { preOrderFlatten, PreOrderFlattenOptions } from '../../../../entity/common';
import { InsightUnit, UnitMatcher } from '../../../../entity/insight';
import { Session } from '../../../../entity/session';
import { getAutoKey } from '../../../../utils/dataAutoKey';
import { isPinned } from '../../unitPin';
/**
 * Searches a list of given @param units recursively in pre-order, comparing them with @param matcher, and save the result path in @path
 *
 * @param units the root units to start searching from
 * @param matcher matcher
 * @param path if a matching unit is found, keeps the path from root to the matching unit.
 * @returns true if found a matching unit
 */
const searchUnit = (units: InsightUnit[], matcher: UnitMatcher, path: InsightUnit[]): boolean => {
    for (const unit of units) {
        path.push(unit);
        if (matcher.target(unit)) {
            return true;
        }
        if (unit.children && searchUnit(unit.children, matcher, path)) {
            return true;
        }
        path.pop();
    }
    return false;
};

type LocateAction = (unit: InsightUnit) => void;

const getTargetUnit = (units: InsightUnit[], matcher: UnitMatcher, callback: LocateAction): InsightUnit | undefined => {
    const path: InsightUnit[] = [];
    if (!searchUnit(units, matcher, path)) {
        return;
    }
    runInAction(() => {
        // expand all parent units
        for (const unit of path) {
            unit.isExpanded = true;
        }
        callback(path[path.length - 1]);
    });
    return path[path.length - 1];
};

const getUnitHeight = (units: InsightUnit[], targetUnit: InsightUnit, options: PreOrderFlattenOptions<InsightUnit>): number => {
    const flattenUnits = preOrderFlatten(units, 0, options);
    let height = 0;
    for (const unit of flattenUnits) {
        if (unit === targetUnit) {
            break;
        }
        height += unit.height();
    }
    height -= 10;
    return height;
};

const getNormalUnitHeight = (session: Session, targetUnit: InsightUnit): number => {
    const when = (unit: InsightUnit): boolean => unit.isExpanded;
    // pinned-by-move-units should be excluded in the result
    const exclude = (unit: InsightUnit): boolean => unit.pinType === 'move' && isPinned(unit);
    const bypass = (unit: InsightUnit): boolean => unit.type === 'transparent';
    return getUnitHeight(session.units, targetUnit, { when, exclude, bypass });
};

export const useJumpTarget = (session: Session, dom: HTMLDivElement | null): void => {
    React.useEffect(() => autorun(
        () => {
            if (session.locateUnit === undefined || dom === null) { return; }
            const targetUnit = getTargetUnit(session.units, session.locateUnit as UnitMatcher, (unit) => {
                session.selectedUnitKeys = [getAutoKey(unit)];
                session.selectedUnits = [unit];
                session.locateUnit?.onSuccess(unit);
            });
            if (targetUnit !== undefined) {
                dom?.scrollTo(0, getNormalUnitHeight(session, targetUnit));
            }
            runInAction(() => {
                session.locateUnit = undefined;
            });
        },
    ), [ session, dom ]);
};
