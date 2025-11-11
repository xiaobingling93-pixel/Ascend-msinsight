/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import { safeJSONParse } from '@insight/lib/utils';
export const chartResize = (ins: echarts.ECharts | null | undefined): void => {
    // ehcarts的custom在渲染超大量数据时会出现残影残留，通过手动触发resize可以消除残影的影响
    if (!ins) {
        return;
    }
    const width = ins.getWidth();
    ins.resize({
        width: width + 1,
    });
    ins.resize({
        width: 'auto',
    });
};
export const generateJsonShow = (text: string): string => {
    if (text === '{}' || text === '') return text;
    const jsonObj = safeJSONParse(text);
    if (jsonObj === null || typeof jsonObj === 'number') return text;
    let res = '';
    Object.keys(jsonObj).forEach((key: string) => { res += `${key}:${jsonObj[key]}\n`; });
    return res;
};
export const convertNanoseconds = (totalNs: number): string => {
    const seconds = Math.floor(totalNs / 1e9);
    const remainingAfterSeconds = totalNs % 1e9;
    const milliseconds = Math.floor(remainingAfterSeconds / 1e6);
    const remainingAfterMilliseconds = remainingAfterSeconds % 1e6;
    const microseconds = Math.floor(remainingAfterMilliseconds / 1e3);
    const nanoseconds = remainingAfterMilliseconds % 1e3;
    return `${seconds}s ${milliseconds.toString().padStart(3, '0')}ms ${microseconds.toString().padStart(3, '0')}us ${nanoseconds.toString().padStart(3, '0')}ns`;
};
