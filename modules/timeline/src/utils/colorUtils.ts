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

// output: "hsl(160, 100%, 80%)" (can be used in canvas directly)
// hsl format: h->[0,360] s->[0, 100] l->[0, 100]
export const hashToColor = (input: string, hueRange: [ number, number ], saturationRange: [ number, number ],
    lightnessRange: [ number, number ]): string => {
    if (hueRange[0] < 0 || saturationRange[0] < 0 || lightnessRange[0] < 0) { return 'hsl(180, 0%, 0%)'; }
    if (hueRange[1] > 360 || saturationRange[1] > 100 || lightnessRange[1] > 100) { return 'hsl(180, 0%, 0%)'; }
    const hue = hashToNumber(input, hueRange[1] - hueRange[0]) + hueRange[0];
    const saturation = hashToNumber(input, saturationRange[1] - saturationRange[0]) + saturationRange[0];
    const lightness = hashToNumber(input, lightnessRange[1] - lightnessRange[0]) + lightnessRange[0];
    return `hsl(${hue}, ${saturation}%, ${lightness}%)`;
};

// hash func
// normal output: [0, maxIndex)
export const hashToNumber = (input: string, maxIndex: number): number => {
    if (maxIndex <= 0) { return 0; }
    let output = 0;
    for (let i = 0; i < input.length; i++) {
        output += 31 * input.charCodeAt(i);
    }
    return Math.abs(output) % maxIndex;
};

// 给频率值加上千位分隔符，比如输入1234567，输出1,234,567
export const freqNumToStr = (num: number): string => {
    let freqStr = String(num);
    const arr = [];
    const count = Math.floor(freqStr.length / 3);
    const commaCount = freqStr.length % 3 === 0 ? count - 1 : count;
    for (let i = 0; i < commaCount; i++) {
        const temp = i === 0 ? freqStr.slice(-3) : freqStr.slice(-3 * (i + 1), -3 * i);
        arr.unshift(temp);
        if (i === commaCount - 1) {
            arr.unshift(freqStr.slice(0, -3 * commaCount));
        }
    }
    if (arr.length > 0) {
        freqStr = arr.join(',');
    }
    return freqStr;
};
