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
import type { InsightUnit } from '../../entity/insight';
const pinState = Symbol('pinnedState');
const sonPinned = Symbol('sonPinned');

export type PinUnit<T extends object> = T & { [pinState]?: boolean; children?: Array<PinUnit<T>>; [sonPinned]?: boolean; parent?: InsightUnit};

export const isPinned = function<T extends object>(unit: PinUnit<T>): boolean {
    if (unit[pinState] === undefined) {
        unit[pinState] = false;
    }
    return unit[pinState];
};

export const isSonPinned = function <T extends object>(unit: PinUnit<T>): boolean {
    if (unit[sonPinned] === undefined) {
        unit[sonPinned] = false;
    }
    return unit[sonPinned];
};

export const isAncestorPinned = function <T extends object>(unit: PinUnit<T>): boolean {
    if (unit.parent === undefined) {
        return false;
    }
    if (isPinned(unit.parent)) {
        return true;
    }
    return isAncestorPinned(unit.parent);
};

export const switchPinned = function<T extends object>(unit: PinUnit<T>): void {
    if (unit[pinState] === undefined) {
        unit[pinState] = false;
    }
    unit[pinState] = !unit[pinState];
    switchSonPinned(unit);
};

const getPinnedFlag = <T extends object>(unit: PinUnit<T>): boolean => {
    if (unit[pinState]) {
        return unit[pinState];
    } else if (unit[sonPinned]) {
        return unit[sonPinned];
    } else {
        return false;
    }
};

function switchSonPinned<T extends object>(unit: PinUnit<T>): void {
    if (unit.children) {
        unit.children.forEach(element => {
            element[sonPinned] = getPinnedFlag(unit);
            switchSonPinned(element);
        });
    }
}
