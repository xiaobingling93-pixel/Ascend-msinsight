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

import { clamp } from 'lodash';
import type { TimeStamp } from '../entity/common';

export type TimeUnit = 'ns' | 'μs' | 'ms' | 's' | 'min' | 'hour' | '';
export interface Time { value: TimeStamp; unit: TimeUnit; length: number };
const getSplitTime = (timestamp: TimeStamp, startTimeUnit: TimeUnit, isNegative: boolean): { steps: Time[]; padder: Time[] } => {
    const timeUnits: TimeUnit[] = ['ns', 'μs', 'ms', 's', 'min', 'hour'];
    let curUnitIdx = timeUnits.indexOf(startTimeUnit);
    const SPLIT_TIME = timeUnits.indexOf('s');
    let curTimeSys = curUnitIdx < SPLIT_TIME ? 1e3 : 60;
    const steps: Time[] = [];
    let tempTimestamp = timestamp;
    do {
        const remainders = tempTimestamp % curTimeSys;
        const unit = timeUnits[curUnitIdx];
        if (curUnitIdx === timeUnits.length - 1) {
            steps.unshift({ value: isNegative ? -tempTimestamp : tempTimestamp, unit, length: tempTimestamp.toString().length });
            break;
        }
        steps.unshift({ value: isNegative ? -remainders : remainders, unit, length: remainders.toString().length });
        tempTimestamp = (tempTimestamp - remainders) / curTimeSys;
        curUnitIdx++;
        curTimeSys = curUnitIdx < SPLIT_TIME ? 1e3 : 60;
    } while (tempTimestamp !== 0);

    const padder: Time[] = [];
    while (curUnitIdx < timeUnits.length - 1) {
        const unit = timeUnits[curUnitIdx];
        padder.unshift({ value: 0, unit, length: 1 });
        curUnitIdx++;
    }

    return { steps, padder };
};

const getPrecisieTail = (splitTimes: Time[], remainderStart: number): number | undefined => {
    const tailer = splitTimes.slice(remainderStart);
    const isEmptyTailer = tailer.filter(timestamp => timestamp.value).length === 0;
    const getTimeSys = (unit: TimeUnit): number => {
        return unit === 'ms' || unit === 'μs' || unit === 'ns' ? 1e3 : 60;
    };
    return isEmptyTailer ? undefined : Math.floor(10 * tailer[0].value / getTimeSys(tailer[0].unit)) / 10;
};

export type GetPadder = (splitTime: Time[], emptyPadder: Time[], padStartIdx: number, timesIdx: number, length?: number) => [ Time[], number ];
const calculateSegments = (steps: Time[], emptyPadder: Time[], segments: number, getPadder: GetPadder): [ Time[], number ] => {
    const splitTime: Time[] = [];
    const reservedDigit = clamp(Math.trunc(segments), 1, steps.length);
    let timesIdx = 0;
    for (; timesIdx < reservedDigit; timesIdx++) {
        splitTime.push(steps[timesIdx]);
    }
    const padStartIdx = clamp(segments - reservedDigit, 0, splitTime.length + emptyPadder.length);
    return getPadder(splitTime, emptyPadder, padStartIdx, timesIdx);
};

export type GetLength = (time?: Time) => number;
const calculateMaxCharlen = (steps: Time[], emptyPadder: Time[], maxCharlen: number, getLength: GetLength, getPadder: GetPadder): [ Time[], number ] => {
    const splitTime: Time[] = [];
    let timesIdx = 0;
    let length = 0;
    length += getLength(steps[timesIdx]);
    do {
        if (getPrecisieTail(steps, timesIdx) === undefined && splitTime.length !== 0) { break; }
        splitTime.push(steps[timesIdx]);
        timesIdx++;
        length += getLength(steps[timesIdx]);
    } while ((length < maxCharlen - 2 && timesIdx < steps.length));
    const emptyPadderIdx = emptyPadder.length - 1;
    return getPadder(splitTime, emptyPadder, emptyPadderIdx, timesIdx, length);
};

export interface TimeOptions {
    // the start time unit
    precision?: TimeUnit;
    // count of the result. eg: segments: 1 -> xx[unit], segments: 2 -> xx[unit1] xx[unit2]...
    segments?: number;
    maxChars?: number;
}
type AdaptTimeArgs = TimeOptions & { getLength: GetLength; getPadder: GetPadder };
export const adaptTime = (time: TimeStamp, {
    precision = 'ns',
    segments,
    maxChars = Number.MAX_SAFE_INTEGER,
    getLength,
    getPadder,
}: AdaptTimeArgs): { splitTime: Time[]; tail?: number } => {
    if (!isFinite(time) || isNaN(time)) { return { splitTime: [{ value: NaN, unit: '', length: precision.length + 1 }] }; };
    let tail;
    const isNegative = time < 0;
    let tempTime = Math.abs(time);
    if (tempTime !== Math.trunc(tempTime)) {
        tail = Math.floor((tempTime - Math.trunc(tempTime)) * 10) / 10;
        isNegative && (tail *= -1);
        tempTime = Math.trunc(tempTime);
    }
    const { steps, padder: emptyPadder } = getSplitTime(tempTime, precision, isNegative);

    let splitTime: Time[] = [];
    let timesIdx = 0;
    if (segments !== undefined) {
        [splitTime, timesIdx] = calculateSegments(steps, emptyPadder, segments, getPadder);
    } else {
        [splitTime, timesIdx] = calculateMaxCharlen(steps, emptyPadder, maxChars, getLength, getPadder);
    }
    tail = getPrecisieTail(steps, timesIdx) ?? tail;
    return { splitTime, tail };
};

export const getLastValue = (isLastItem: boolean, value: number, tail?: number): string | number => {
    if (isLastItem) {
        if (tail === 0) {
            return value.toFixed(1);
        }
        if (tail !== undefined) {
            return value + tail;
        }
    }
    return value;
};

export const isLowerUnit = (time: Time): boolean => {
    return time?.unit === 'ms' || time?.unit === 'μs' || time?.unit === 'ns';
};
