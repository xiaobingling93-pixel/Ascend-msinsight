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

export class SwitchCacheInPhase implements Cache {
    phase: ValidSession['phase'];
    cache: Cache;
    factory: CacheFactory;
    switched: boolean = false;

    constructor(phase: ValidSession['phase'], factory: CacheFactory) {
        this.phase = phase;
        this.factory = factory;
        this.cache = factory.create();
    }

    getData<T extends any>(session: ValidSession, params: any): Promise<any> {
        if (!this.switched && session.phase === this.phase) {
            this.cache = this.factory.create();
            this.switched = true;
        }
        return this.cache.getData(session, params);
    }
};
