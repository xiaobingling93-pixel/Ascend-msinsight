/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
