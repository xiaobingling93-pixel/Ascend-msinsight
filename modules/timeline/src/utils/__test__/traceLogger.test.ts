/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { traceStart, traceEnd, traceSingle } from '../traceLogger'; // 假设文件名为 trace.ts
import { platform } from '../../platforms';

// Mock the platform.trace function
jest.mock('../../platforms', () => ({
    platform: {
        trace: jest.fn(),
    },
}));

describe('trace functions', () => {
    test('traceEnd should update the trace entry and call platform.trace', () => {
        const key = 'testKey';
        const infos = { action: 'testAction' };
        traceStart(key, infos);
        traceEnd(key);
        expect(platform.trace).toHaveBeenCalledWith(infos.action, expect.objectContaining({
            responseTime: expect.any(Number),
        }));
    });

    test('traceEnd should not call platform.trace if the key does not exist', () => {
        const key = 'nonExistentKey';
        traceEnd(key);
        expect(platform.trace).not.toHaveBeenCalled();
    });

    test('traceSingle should call platform.trace with an empty object for noneResEvent actions', () => {
        const action = 'selectJsLane';
        traceSingle(action, ['unit1', 'unit2']);
        expect(platform.trace).toHaveBeenCalledWith(action, {});
    });

    test('traceSingle should call platform.trace with units for non-noneResEvent actions', () => {
        const action = 'otherAction';
        const unitNames = ['unit1', 'unit2'];
        traceSingle(action, unitNames);
        expect(platform.trace).toHaveBeenCalledWith(action, { units: unitNames });
    });
});
