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
import { Session } from '../entity/session';

export class SessionStore {
    private _activeSession: Session;
    constructor() {
        makeAutoObservable(this);
        this._activeSession = new Session();
    }

    /**
     * The session store is locked if a session is in recording, users can't switch between sessions when session store is locked.
     * @returns Whether session store is locked
     */
    get activeSession(): Session {
        return this._activeSession;
    }

    set activeSession(value: Session) {
        this._activeSession = value;
    }

    async newSession(conf?: Partial<Session> | Session): Promise<Session > {
        const session = new Session();
        return session;
    }
}
