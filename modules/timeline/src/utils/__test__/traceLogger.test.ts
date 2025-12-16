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
