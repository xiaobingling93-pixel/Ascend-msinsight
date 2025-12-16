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
import type { DataMatcher, Session } from '../../../entity/session';
import { type InsightUnit } from '../../../entity/insight';
import { isPinned } from '../../ChartContainer/unitPin';

export const getAllNormalUnits = (units: InsightUnit[]): InsightUnit[] => {
    return units.flatMap((unit) => {
        if (unit.isExpanded && unit.children) {
            return [unit, ...getAllNormalUnits(unit.children)];
        }
        return unit;
    });
};

export const UNDRAW_HEIGHT = 45;
export const getHeight = (session: Session, matcher: DataMatcher, findData: Record<string, unknown>): number => {
    const searchData = (unit: InsightUnit, exclude: boolean): number => {
        if (matcher(unit) && !exclude) {
            // 补丁,drawData.height未定义时坐标为0的bug
            if (!Array.isArray(unit.chart) && typeof findData.depth === 'number') {
                findData.height = ((unit.chart?.config as any)?.rowHeight ?? 0) * ((2 * findData.depth) + 1) / 2;
            } else {
                findData.height = 0;
            }
            return curHeight + (findData.height as number);
        }
        return 0;
    };
    let curHeight = UNDRAW_HEIGHT;
    const { pinnedUnits } = session;
    for (const unit of pinnedUnits) {
        const dataHeight = searchData(unit, false);
        if (dataHeight !== 0) {
            return dataHeight;
        }
        curHeight += unit.height() + 1;
    }
    curHeight -= session.scrollTop;

    const allNormalUnits = getAllNormalUnits(session.units);
    for (const unit of allNormalUnits) {
        const isPinnedUnit = isPinned(unit);
        const dataHeight = searchData(unit, isPinnedUnit);
        if (dataHeight !== 0) {
            return dataHeight;
        }
        if (!isPinnedUnit) {
            curHeight += unit.height() + 1;
        }
    }
    return 0;
};

// 当连线不包含置顶泳道时，裁切掉置顶泳道以上的连线部分
export function calculateClipTopAndPinedHeight<T, U>(pinnedUnits: InsightUnit[],
    target: { data: T; matcher: DataMatcher }, sources: Array<{ data: U; matcher: DataMatcher }>): [clipTop: number, pinnedHeight: number] {
    const pinnedHeight = pinnedUnits.reduce((acc, unit) => acc + unit.height() + 1, 0);
    let clipTop = UNDRAW_HEIGHT;
    const matchers = [target.matcher].concat(sources.map(source => source.matcher));
    const hasPined = pinnedUnits.some(unit => matchers.some(matcher => matcher(unit)));
    if (!hasPined) { clipTop += pinnedHeight; }
    return [clipTop, pinnedHeight];
}
