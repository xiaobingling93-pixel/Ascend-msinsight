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

/**
 * binarySearch
 * @param arr data
 * @param getValue computes a value from a given data for comparison
 * @param target target
 * @returns index
 */
export function binarySearchFirstBig<T>(arr: T[], getValue: (elem: T) => number, target: number): number {
    if (arr.length <= 1) { return 0; }
    let lowIndex = 0;
    let highIndex = arr.length - 1;

    while (lowIndex <= highIndex) {
        const midIndex = Math.floor((lowIndex + highIndex) / 2);
        if (getValue(arr[midIndex]) >= target) {
            if (midIndex === 0 || getValue(arr[midIndex - 1]) < target) { return midIndex; }
            highIndex = midIndex - 1;
        } else {
            lowIndex = midIndex + 1;
        }
    }
    return arr.length - 1;
}

/**
 * binarySearch
 * @param arr data
 * @param getValue computes a value from a given data for comparison
 * @param target target
 * @returns index
 */
export function binarySearchLastSmall<T>(arr: T[], getValue: (elem: T) => number, target: number): number {
    if (arr.length <= 1) { return 0; }
    let lowIndex = 0;
    let highIndex = arr.length - 1;

    while (lowIndex <= highIndex) {
        const midIndex = Math.floor((lowIndex + highIndex) / 2);
        if (getValue(arr[midIndex]) <= target) {
            if (midIndex === arr.length - 1 || getValue(arr[midIndex + 1]) > target) { return midIndex; }
            lowIndex = midIndex + 1;
        } else {
            highIndex = midIndex - 1;
        }
    }
    return 0;
}

/**
 * binarySearchLastSmall function adaptation, processing basic data types.
 *
 * The same implementation function can jump to the getTime function in the TimeSeriesCache class,
 * which obtains a property value in the object.
 * @param e
 */
export function self<T>(e: T): T {
    return e;
}
