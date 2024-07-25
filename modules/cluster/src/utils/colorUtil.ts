/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { Theme } from '@emotion/react';
export const colorPalette: Array<keyof Theme['colorPalette']> = [
    'deepBlue',
    'coralRed',
    'tealGreen',
    'aquaBlue',
    'raspberryPink',
    'vividBlue',
    'vividRed',
    'royalPurple',
    'skyBlue',
    'sunsetOrange',
    'amethystPurple',
    'limeGreen',
];
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
