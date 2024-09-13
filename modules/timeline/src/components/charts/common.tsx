/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import styled from '@emotion/styled';

// rect: [x, y, width, height]
export const drawRoundedRect = (rect: number[], context: CanvasRenderingContext2D, bottomRouned: number, topRounded?: number): void => {
    const points = [
        [rect[0] + (topRounded ?? bottomRouned), rect[1]],
        [rect[0] + rect[2], rect[1]],
        [rect[0] + rect[2], rect[1] + rect[3]],
        [rect[0], rect[1] + rect[3]],
        [rect[0], rect[1]],
    ];
    context.beginPath();
    context.moveTo(points[0][0], points[0][1]);
    // draw top-right
    context.arcTo(points[1][0], points[1][1], points[2][0], points[2][1], topRounded ?? bottomRouned);
    // draw bottom-right
    context.arcTo(points[2][0], points[2][1], points[3][0], points[3][1], bottomRouned);
    // draw bottom-left
    context.arcTo(points[3][0], points[3][1], points[4][0], points[4][1], bottomRouned);
    // draw top-left
    context.arcTo(points[4][0], points[4][1], points[0][0], points[0][1], topRounded ?? bottomRouned);
    context.closePath();
};

export const drawMultiBgRoundedRect = (rect: number[], context: CanvasRenderingContext2D, radius: number, order: number): void => {
    let points;
    if (order === 0) {
        // 左边圆弧，右边直角，初始点右上角
        points = [
            [rect[0] + rect[2], rect[1]],
            [rect[0], rect[1]],
            [rect[0], rect[1] + rect[3]],
            [rect[0] + rect[2], rect[1] + rect[3]],
        ];
    } else {
        // 右边圆弧，左边直角，初始点左上角
        points = [
            [rect[0], rect[1]],
            [rect[0] + rect[2], rect[1]],
            [rect[0] + rect[2], rect[1] + rect[3]],
            [rect[0], rect[1] + rect[3]],
        ];
    }
    context.beginPath();
    context.moveTo(points[0][0], points[0][1]);
    context.arcTo(points[1][0], points[1][1], points[2][0], points[2][1], radius);
    context.arcTo(points[2][0], points[2][1], points[3][0], points[3][1], radius);
    context.lineTo(points[3][0], points[3][1]);
    context.lineTo(points[0][0], points[0][1]);
    context.closePath();
};

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
    left: 0px;
    bottom: 0px;
    display: flex;
    flex-direction: row-reverse;
`;
