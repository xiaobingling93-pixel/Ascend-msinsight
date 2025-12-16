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
import { InsightStore } from './insight';
import { SessionStore } from './session';

export class RootStore {
    insightStore: InsightStore;
    sessionStore: SessionStore;

    constructor() {
        this.insightStore = new InsightStore();
        this.sessionStore = new SessionStore();
    }

    resetStore = (): void => {
        this.insightStore = new InsightStore();
        this.sessionStore = new SessionStore();
    };
}

export const store = new RootStore();
