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
import { getDuration } from '../../../utils/humanReadable';
import type { XReverseScaleRef } from './ChartInteractor';

export interface Pos {
    x: number;
    y: number;
    absoluteX: number;
    absoluteY: number;
};

export interface ExtendPos extends Pos {
    timeAxisX: number;
}

const TEXT_WIDTH = 70;
const FONT_SIZE = 12;

export const getTimeDifference = (time1: number, time2: number, isNsMode: boolean): string => {
    const timeStr = getDuration(Math.abs(time1 - time2), { precision: isNsMode ? 'ns' : 'ms', maxChars: TEXT_WIDTH / FONT_SIZE });
    return timeStr;
};

export const SINGLE_DRAG_OFFSET = 2;
export const isOnSideline = (mousePos: Pos | undefined, selectedRange: [ number, number ] | undefined, xReverseScaleRef: XReverseScaleRef): boolean => {
    if (selectedRange === undefined || !mousePos) {
        return false;
    }
    const offsetX = mousePos.x;
    const isOnSideLine = ((offsetX <= xReverseScaleRef.current(selectedRange[0]) + SINGLE_DRAG_OFFSET &&
            offsetX >= xReverseScaleRef.current(selectedRange[0]) - SINGLE_DRAG_OFFSET) ||
        (offsetX <= xReverseScaleRef.current(selectedRange[1]) + SINGLE_DRAG_OFFSET &&
            offsetX >= xReverseScaleRef.current(selectedRange[1]) - SINGLE_DRAG_OFFSET));
    if (isOnSideLine) {
        return true;
    }
    return false;
};
