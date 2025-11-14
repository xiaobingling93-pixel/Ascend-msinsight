/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { makeAutoObservable } from 'mobx';
import { Session } from '../entity/session';

export class SessionStore {
    private _activeSession: Session | undefined;

    constructor() {
        makeAutoObservable(this);
        this._activeSession = new Session({
            id: 'entry',
            name: 'entry',
            phase: 'configuring',
            units: [],
            availableUnits: [],
            startRecordTime: 0,
            endTimeAll: undefined,
            isNsMode: true,
            isOverflowMaxSafeNumber: false,
        });
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
        const session = new Session({
            id: 'entry',
            name: conf?.name,
            phase: conf?.phase ?? 'configuring',
            units: conf?.units ?? [],
            availableUnits: conf?.availableUnits ?? [],
            icon: conf?.icon,
            startRecordTime: conf?.startRecordTime,
            endTimeAll: conf?.endTimeAll,
            isNsMode: conf?.isNsMode,
            isOverflowMaxSafeNumber: false,
        });
        return session;
    }
}
