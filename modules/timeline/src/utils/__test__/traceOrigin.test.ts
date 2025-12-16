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
