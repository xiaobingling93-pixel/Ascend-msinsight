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

import { RootStore } from './rootStore';
import { SessionStore } from './sessionStore';
import { MemoryStore } from './memoryStore';

describe('RootStore', () => {
    let rootStore: RootStore;

    beforeEach(() => {
        rootStore = new RootStore();
    });

    it('should initialize with SessionStore and MemoryStore', () => {
        expect(rootStore.sessionStore).toBeInstanceOf(SessionStore);
        expect(rootStore.memoryStore).toBeInstanceOf(MemoryStore);
    });

    it('should reset the stores', () => {
        const originalSessionStore = rootStore.sessionStore;
        const originalMemoryStore = rootStore.memoryStore;

        rootStore.resetStore();

        expect(rootStore.sessionStore).not.toBe(originalSessionStore);
        expect(rootStore.memoryStore).not.toBe(originalMemoryStore);
        expect(rootStore.sessionStore).toBeInstanceOf(SessionStore);
        expect(rootStore.memoryStore).toBeInstanceOf(MemoryStore);
    });
});
