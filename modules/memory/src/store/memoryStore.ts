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
