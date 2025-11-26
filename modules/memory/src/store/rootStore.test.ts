/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
