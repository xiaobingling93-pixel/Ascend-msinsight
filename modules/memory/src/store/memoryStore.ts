/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { makeAutoObservable } from 'mobx';
import { MemorySession } from '../entity/memorySession';

export class MemoryStore {
    private _activeSession: MemorySession | undefined;

    constructor() {
        makeAutoObservable(this);
        this._activeSession = new MemorySession();
    }

    /**
     * The session store is locked if a session is in recording, users can't switch between sessions when session store is locked.
     *
     * @returns Whether session store is locked
     */
    get activeSession(): MemorySession | undefined {
        return this._activeSession;
    }

    set activeSession(value: MemorySession | undefined) {
        this._activeSession = value;
    }

    // creates a new session in the store.
    async newSession(conf?: Partial<MemorySession> | MemorySession): Promise<MemorySession | undefined> {
        const session = new MemorySession();
        return session;
    }
};
