/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { SessionStore } from './sessionStore';
import { MemoryStore } from './memoryStore';

export class RootStore {
    sessionStore: SessionStore;
    memoryStore: MemoryStore;

    constructor() {
        this.sessionStore = new SessionStore();
        this.memoryStore = new MemoryStore();
    }

    resetStore = (): void => {
        this.sessionStore = new SessionStore();
        this.memoryStore = new MemoryStore();
    };
}

export const store = new RootStore();
