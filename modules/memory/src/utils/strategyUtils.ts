/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

/**
 * Memory顶部筛选条件策略接口
 */
export interface MemoryHeaderStrategy {
    shouldDisplay: (field: string) => boolean;
};

export class NormalDisplayStrategy implements MemoryHeaderStrategy {
    shouldDisplay = (field: string): boolean => {
        const fullDisplayFields = ['rankId', 'groupId'];
        return fullDisplayFields.includes(field);
    };
};

export class FullDisplayStrategy implements MemoryHeaderStrategy {
    shouldDisplay = (field: string): boolean => {
        const fullDisplayFields = ['host', 'rankId', 'groupId'];
        return fullDisplayFields.includes(field);
    };
};

export class SingleDisplayStrategy implements MemoryHeaderStrategy {
    shouldDisplay = (field: string): boolean => {
        const fullDisplayFields = ['rankId'];
        return fullDisplayFields.includes(field);
    };
};

export class HostCompareDisplayStrategy implements MemoryHeaderStrategy {
    shouldDisplay = (field: string): boolean => {
        const fullDisplayFields = ['host', 'rankId'];
        return fullDisplayFields.includes(field);
    };
};

export const displayStrategyKey = {
    resourceType: {
        pytorch: 'pytorch',
        mindspore: 'mindspore',
    },
    hasHostOptions: {
        isHost: 'isHost',
        notHost: 'notHost',
    },
    isComparing: {
        isCompared: 'isCompared',
        notCompared: 'notCompared',
    },
};

export const displayStrategyMap: {
    [resourceType: string]: {
        [hasHostOptions: string]: {
            [isComparing: string]: MemoryHeaderStrategy;
        };
    };
} = {
    pytorch: {
        isHost: {
            isCompared: new FullDisplayStrategy(),
            notCompared: new FullDisplayStrategy(),
        },
        notHost: {
            isCompared: new NormalDisplayStrategy(),
            notCompared: new NormalDisplayStrategy(),
        },
    },
    mindspore: {
        isHost: {
            isCompared: new FullDisplayStrategy(),
            notCompared: new FullDisplayStrategy(),
        },
        notHost: {
            isCompared: new NormalDisplayStrategy(),
            notCompared: new NormalDisplayStrategy(),
        },
    },
};
