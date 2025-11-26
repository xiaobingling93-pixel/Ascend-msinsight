/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import { SessionStore } from './sessionStore';
import { Session } from '../entity/session';

describe('SessionStore', () => {
    let sessionStore: SessionStore;

    beforeEach(() => {
        sessionStore = new SessionStore();
    });

    it('should initialize with an active session', () => {
        expect(sessionStore.activeSession).toBeInstanceOf(Session);
    });

    it('should allow setting and getting the active session', () => {
        const newSession = new Session();
        sessionStore.activeSession = newSession;
        expect(sessionStore.activeSession).toBe(newSession);
    });

    it('should create a new session', async () => {
        const newSession = await sessionStore.newSession();
        expect(newSession).toBeInstanceOf(Session);
    });

    it('should create a new session with configuration', async () => {
        const conf = { unitcount: 2 };
        const newSession = await sessionStore.newSession(conf);
        expect(newSession).toBeInstanceOf(Session);
        expect(newSession?.unitcount).toBe(0);
    });
});
