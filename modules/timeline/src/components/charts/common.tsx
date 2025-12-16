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

import styled from '@emotion/styled';

/**
 * Limits the data size to make sure that every data held in the chart are actually drawn, ie. no wasted rendering
 *
 * @param originalData the original data, limited to [ domainStart, domainEnd ]
 * @param width width of the canvas, in px
 * @param start start time
 * @param end end time
 * @returns the data set that can all be actually rendered.
 */
export const zipStatusData = <T extends { startTime: number; duration: number; type: string }>(originalData: T[],
    width: number, start: number, end: number): T[] => {
    return originalData;
};

export const zipTimeSeriesData = (dataset: number[][], width: number, start: number, end: number): number[][] => {
    if (width <= 0 || dataset.length === 0 || width === Infinity) {
        return [];
    }
    const pxToTime = (px: number): number => start + ((end - start) / width * px);
    const getTimestamp = (data: number[]): number => data[0];

    const result: number[][] = [];
    let dataIndex = 0;
    let rangeStart = pxToTime(0);
    let rangeEnd: number;
    if (getTimestamp(dataset[0]) < rangeStart) {
        // add the first data point even if it's out of render domain.
        // the caller should ensure that there's only one data that occurs earlier than the render domain
        result.push(dataset[0]);
    }
    // the last data occured in the previous pixels, whose values are kept until the next data point occur
    let lastData = dataset[0].map(_ => 0);
    let isNewDataOccurred = false; // used for optimization
    for (let pxIndex = 0; pxIndex < width; pxIndex++) {
        if (dataIndex === dataset.length) {
            break;
        }
        rangeEnd = pxToTime(pxIndex + 1);

        // no new data in this pixel, copy the last occured data values
        if (getTimestamp(dataset[dataIndex]) > rangeEnd) {
            if (isNewDataOccurred) {
                result.push([rangeStart, ...lastData.slice(1)]);
            }
            isNewDataOccurred = false;
            // prepare for next loop
            rangeStart = rangeEnd;
            continue;
        }

        const max = dataset[dataIndex].slice(1); // the max values of each sub-data in this pixel
        do {
            lastData = dataset[dataIndex];
            max.forEach((it, i, arr) => {
                if (dataset[dataIndex][i + 1] > it) {
                    arr[i] = dataset[dataIndex][i + 1];
                }
            });
            dataIndex++;
        } while (dataIndex < dataset.length && getTimestamp(dataset[dataIndex]) <= rangeEnd);
        result.push([rangeStart, ...max]);
        isNewDataOccurred = true;
        // prepare for next loop
        rangeStart = rangeEnd;
    }
    // take the last data even if it's out of the render domain
    if (getTimestamp(dataset[dataset.length - 1]) > end) {
        result.push(dataset[dataset.length - 1]);
    }
    return result;
};

// find the last data that has key <= target
export const search = <T extends number | object>(data: T[], target: number, getKey: (elem: T) => number,
    range: [number, number] = [0, data.length]): number => {
    let [lo, hi] = range;
    while (lo < hi - 1) {
        const mid = Math.floor((lo + hi) / 2);
        const key = getKey(data[mid]);
        if (target < key) {
            hi = mid;
        } else if (target > key) {
            lo = mid;
        } else {
            lo = mid;
            while (lo + 1 < data.length && getKey(data[lo + 1]) === target) {
                lo++;
            }
            return lo;
        }
    }
    return lo;
};

export const CanvasContainer = styled.div<{ width: number; height: number }>`
    position: relative;
    width: ${(p): number => p.width}px;
    height: ${(p): number => p.height}px;
`;

export const SkeletonWrapper = styled.div`
    display: flex;
    align-items: center;   /* 垂直居中 */
    justify-content: center; /* 若骨架宽度较小，水平也居中 */
    width: 100%;
    height: 100%;
`;

export const Canvas = styled.canvas`
    width: 100%;
    height: 100%;
    margin: 0;
    padding: 0;
`;

export const SVG = styled.svg`
    margin: 0;
    padding: 0;
    position: absolute;
    top: 0;
    bottom: 0;
    left: 0;
    right: 0;
    width: 100%;
    height: 100%;
`;

export const LegendArea = styled.div`
    margin: 0;
    padding: 0;
    position: absolute;
    top: 4px;
    right: 16px;
    left: 0;
    bottom: 0;
    display: flex;
    flex-direction: row-reverse;
`;
