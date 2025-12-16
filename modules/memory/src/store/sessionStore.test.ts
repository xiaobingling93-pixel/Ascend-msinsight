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
