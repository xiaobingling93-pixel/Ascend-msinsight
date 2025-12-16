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
import type { ValidSession } from '../../entity/session';
import type { Cache, CacheFactory } from '../cache';
import type { KeyedCacheFactory } from './simple';

/**
 * session: {
 *   caches: {
*      // mapByParam 生成
 *     [dataKey]: {
 *       unit0,
 *       unit1,
 *       ...
 *     },
 *     ...otherCaches,
 *   }
 * }
 * Cache 是由 dataKey 管理
 * 某些 dataKey 下会存在多个相同类型的 unit
 * mapByParam 会生成一张 map 用于管理相同 dataKey 下的不同 unit
 * 不同 unit 发送的请求必须能映射到不同的 key
 * paramToKey 是一个用来获取这个 key 值的函数
 *
 * @param key dataKey
 * @param paramToKey get key from request param
 * @param cacheFactory cache strategy
 * @returns a cache backed by a map.
 */
export const mapByParam = <P, K extends keyof any>(key: K, paramToKey: (param: any) => P, cacheFactory: KeyedCacheFactory<K>): Cache => {
    return new ParamMappedCache(key, paramToKey, cacheFactory);
};

class ParamMappedCache<P, K extends keyof any> implements Cache {
    private readonly caches = new Map<P, Cache>();
    private readonly key: keyof any;
    private readonly factory: CacheFactory;
    private readonly paramToKey: (param: any) => P;

    constructor(key: K, paramToKey: (param: any) => P, factory: KeyedCacheFactory<K>) {
        this.key = key;
        this.factory = factory;
        this.paramToKey = paramToKey;
    }

    async getData<T extends any>(session: ValidSession, params: any): Promise<any> {
        const requestParam = params[this.key as unknown as keyof any] as unknown as any;
        const requestKey = this.paramToKey(requestParam);
        const targetCache = this.caches.get(requestKey);
        if (targetCache) {
            return targetCache.getData(session, params);
        }
        const newCacheEntry = this.factory.create();
        this.caches.set(requestKey, newCacheEntry);
        return newCacheEntry.getData(session, params);
    }
}
