/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
export const CACHELINE_RECORD = 'Cacheline Records';
export const CACHELINE_ID = 'Cacheline Id';
export const HIT = 'Hit';
export const MISS = 'Miss';
export const ADDRESS_RANGE = 'Address Range';
const VALUE = 'Value';

export interface QueryCacheRecordReturn {
    [CACHELINE_RECORD]: CacheRecordItem[];
};

export interface CacheEvent {
    [ADDRESS_RANGE]: string[][];
    [VALUE]: number[];
}

export type CacheEventType = 'Hit' | 'Miss';

export interface CacheRecordItem {
    [p: string]: any;
    [CACHELINE_ID]: number;
    [HIT]: CacheEvent;
    [MISS]: CacheEvent;
};
