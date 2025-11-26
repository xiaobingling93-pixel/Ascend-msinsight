/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
