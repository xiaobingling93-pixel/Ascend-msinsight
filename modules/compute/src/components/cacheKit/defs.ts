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
