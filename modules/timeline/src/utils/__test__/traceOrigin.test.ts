/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { getOrigin, WithOrigin } from '../traceOrigin'; // 替换为你的文件路径

describe('getOrigin', () => {
    it('should return the original object if no origin is set', () => {
        const obj = { key: 'value' };
        const result = getOrigin(obj);
        expect(result).toBe(obj);
    });

    it('should return the origin object if it is set', () => {
        const obj = { key: 'value' };
        const originObj = { differentKey: 'differentValue' };
        const withOrigin: WithOrigin<typeof obj> = { ...obj, [Symbol('hhh')]: originObj };
        const result = getOrigin(withOrigin);
        expect(result).toBe(withOrigin);
    });
});
