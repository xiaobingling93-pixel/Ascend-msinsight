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

import { savePageSetting, recoverPageSetting, updatePageSetting } from '../PageSetting';
import { store } from '../../store';
import { InsightUnit } from '../../entity/insight';
import { Session } from '../../entity/session';
// 模拟 store 和 session
jest.mock('../../store', () => ({
    store: {
        sessionStore: {
            activeSession: {
                units: [],
                domainRange: {},
                pageSetting: {},
                projectName: 'testProject',
            },
        },
    },
}));

beforeEach(() => {
    store.sessionStore.activeSession = {
        units: [],
        domainRange: {
            domainStart: 0,
            domainEnd: 0,
        },
        pageSetting: {},
        projectName: 'testProject',
    } as Partial<Session> as Session;
});

describe('savePageSetting', () => {
    it('should save page setting when session is valid', () => {
        const session = store.sessionStore.activeSession;
        expect(session).toBeDefined();
        if (session === undefined) {
            return;
        }
        const mockUnit = {
            isExpanded: true,
            children: [],
        } as Partial<InsightUnit> as InsightUnit;
        session.units = [mockUnit];
        savePageSetting();
        if (session.projectName === undefined) {
            return;
        }
        expect(session.pageSetting[session.projectName]).toBeDefined();
    });

    it('should not save page setting when session is undefined', () => {
        store.sessionStore.activeSession = undefined;
        savePageSetting();
        expect(store.sessionStore.activeSession).toBeUndefined();
    });

    it('should not save page setting when units are empty', () => {
        const session = store.sessionStore.activeSession;
        expect(session).toBeDefined();
        if (session === undefined) {
            return;
        }
        session.units = [];
        savePageSetting();
        if (session.projectName === undefined) {
            return;
        }
        expect(session.pageSetting[session.projectName]).toBeUndefined();
    });
});

describe('recoverPageSetting', () => {
    it('should recover page setting when session is valid', () => {
        const session = store.sessionStore.activeSession;
        expect(session).toBeDefined();
        expect(session?.projectName).toBeDefined();
        if (session === undefined || session.projectName === undefined) {
            return;
        }
        session.pageSetting[session.projectName] = {
            units: [{ isExpanded: true, children: [] }],
            domainRange: {
                domainStart: 1,
                domainEnd: 0,
            },
        };
        recoverPageSetting();
        expect(session.domainRange).toEqual({
            domainStart: 1,
            domainEnd: 0,
        });
    });

    it('should not recover page setting when session is undefined', () => {
        store.sessionStore.activeSession = undefined;
        recoverPageSetting();
        expect(store.sessionStore.activeSession).toBeUndefined();
    });
});

describe('updatePageSetting', () => {
    it('should update project name in page setting', () => {
        const session = store.sessionStore.activeSession;
        expect(session).toBeDefined();
        if (session === undefined) {
            return;
        }
        session.pageSetting.oldProjectName = {
            units: [{ isExpanded: true, children: [] }],
            domainRange: {
                domainStart: 0,
                domainEnd: 0,
            },
        };
        updatePageSetting({
            type: 'updateProjectName',
            data: { oldProjectName: 'oldProjectName', newProjectName: 'newProjectName' },
        });
        expect(session.pageSetting.newProjectName).toBeDefined();
        expect(session.pageSetting.oldProjectName).toBeUndefined();
    });

    it('should remove single data path from page setting', () => {
        const session = store.sessionStore.activeSession;
        expect(session).toBeDefined();
        if (session === undefined) {
            return;
        }
        session.pageSetting.testProject = {
            units: [{ isExpanded: true, children: [] }],
            domainRange: {
                domainStart: 0,
                domainEnd: 0,
            },
        };
        updatePageSetting({
            type: 'removeSingleDataPath',
            data: { singleDataPath: 'testPath', dataSource: { dataPath: ['testPath'], projectName: 'testProject' } },
        });
        expect(session.pageSetting.testProject.units.length).toBe(0);
    });

    it('should remove data source from page setting', () => {
        const session = store.sessionStore.activeSession;
        expect(session).toBeDefined();
        if (session === undefined) {
            return;
        }
        session.pageSetting.testProject = {
            units: [{ isExpanded: true, children: [] }],
            domainRange: {
                domainStart: 0,
                domainEnd: 0,
            },
        };
        updatePageSetting({ type: 'removeDataSource', data: { projectName: 'testProject' } });
        expect(session.pageSetting.testProject).toBeUndefined();
    });

    it('should reset page setting', () => {
        const session = store.sessionStore.activeSession;
        expect(session).toBeDefined();
        if (session === undefined) {
            return;
        }
        session.pageSetting.testProject = {
            units: [{ isExpanded: true, children: [] }],
            domainRange: {
                domainStart: 0,
                domainEnd: 0,
            },
        };
        updatePageSetting({ type: 'reset' });
        expect(Object.keys(session.pageSetting).length).toBe(0);
    });
});
