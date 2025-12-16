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

import { MemoryStore } from './memoryStore';
import { MemorySession } from '../entity/memorySession';

describe('MemoryStore', () => {
    let memoryStore: MemoryStore;

    beforeEach(() => {
        memoryStore = new MemoryStore();
    });

    it('should initialize with an active session', () => {
        expect(memoryStore.activeSession).toBeInstanceOf(MemorySession);
    });

    it('should allow setting and getting the active session', () => {
        const newSession = new MemorySession();
        memoryStore.activeSession = newSession;
        expect(memoryStore.activeSession).toBe(newSession);
    });

    it('should create a new session', async () => {
        const newSession = await memoryStore.newSession();
        expect(newSession).toBeInstanceOf(MemorySession);
    });

    it('should create a new session with configuration', async () => {
        const conf = { memoryGraphId: '2' };
        const newSession = await memoryStore.newSession(conf);
        expect(newSession).toBeInstanceOf(MemorySession);
        expect(newSession?.memoryGraphId).toBe('');
    });
});
