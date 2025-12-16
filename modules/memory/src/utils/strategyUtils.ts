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
