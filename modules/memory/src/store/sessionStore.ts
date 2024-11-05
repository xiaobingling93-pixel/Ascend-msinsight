/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { makeAutoObservable } from 'mobx';
import { Session } from '../entity/session';

export class SessionStore {
    private _activeSession: Session | undefined;

    constructor() {
        makeAutoObservable(this);
        this._activeSession = new Session();
    }

    /**
     * The session store is locked if a session is in recording, users can't switch between sessions when session store is locked.
     *
     * @returns Whether session store is locked
     */
    get activeSession(): Session | undefined {
        return this._activeSession;
    }

    set activeSession(value: Session | undefined) {
        this._activeSession = value;
    }

    // creates a new session in the store.
    async newSession(conf?: Partial<Session> | Session): Promise<Session | undefined> {
        const session = new Session();
        return session;
    }
}
