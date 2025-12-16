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
import {
    NormalDisplayStrategy,
    FullDisplayStrategy,
    SingleDisplayStrategy,
    HostCompareDisplayStrategy,
    displayStrategyMap,
} from '../strategyUtils';

describe('MemoryHeaderStrategy Implementations', () => {
    it('NormalDisplayStrategy should display rankId and groupId', () => {
        const strategy = new NormalDisplayStrategy();
        expect(strategy.shouldDisplay('rankId')).toBe(true);
        expect(strategy.shouldDisplay('groupId')).toBe(true);
        expect(strategy.shouldDisplay('host')).toBe(false);
    });

    it('FullDisplayStrategy should display host, rankId, and groupId', () => {
        const strategy = new FullDisplayStrategy();
        expect(strategy.shouldDisplay('host')).toBe(true);
        expect(strategy.shouldDisplay('rankId')).toBe(true);
        expect(strategy.shouldDisplay('groupId')).toBe(true);
    });

    it('SingleDisplayStrategy should only display rankId', () => {
        const strategy = new SingleDisplayStrategy();
        expect(strategy.shouldDisplay('rankId')).toBe(true);
        expect(strategy.shouldDisplay('groupId')).toBe(false);
        expect(strategy.shouldDisplay('host')).toBe(false);
    });

    it('HostCompareDisplayStrategy should display host and rankId', () => {
        const strategy = new HostCompareDisplayStrategy();
        expect(strategy.shouldDisplay('host')).toBe(true);
        expect(strategy.shouldDisplay('rankId')).toBe(true);
        expect(strategy.shouldDisplay('groupId')).toBe(false);
    });
});

describe('Display Strategy Map', () => {
    it('should return FullDisplayStrategy for pytorch with isHost and isCompared', () => {
        const strategy = displayStrategyMap.pytorch.isHost.isCompared;
        expect(strategy.shouldDisplay('host')).toBe(true);
        expect(strategy.shouldDisplay('rankId')).toBe(true);
        expect(strategy.shouldDisplay('groupId')).toBe(true);
    });

    it('should return NormalDisplayStrategy for pytorch with notHost and notCompared', () => {
        const strategy = displayStrategyMap.pytorch.notHost.notCompared;
        expect(strategy.shouldDisplay('rankId')).toBe(true);
        expect(strategy.shouldDisplay('groupId')).toBe(true);
        expect(strategy.shouldDisplay('host')).toBe(false);
    });

    // Add more tests for other combinations if needed
});
