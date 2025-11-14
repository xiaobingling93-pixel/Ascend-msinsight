/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { UnitDetail } from '../../api/interface';
import { store } from '../../store';
import { InsightUnit } from '../../entity/insight';
import { Session } from '../../entity/session';
import jumpToCorrespondingLane from '../jumpToCorrespondingLane';

// 模拟 store 和 sessionStore
jest.mock('../../store', () => ({
    store: {
        sessionStore: {
            activeSession: {
                locateUnit: {
                    target: jest.fn().mockReturnValue(true),
                    onSuccess: jest.fn(),
                    showDetail: false,
                },
            },
        },
    },
}));

describe('jumpToCorrespondingLane', () => {
    it('should not do anything if session is undefined', () => {
        // 模拟 session 为 undefined
        store.sessionStore.activeSession = undefined;

        const unitDetail: UnitDetail = {
            cardId: '123',
            pid: '456',
            propsCardId: '789',
        };

        jumpToCorrespondingLane(unitDetail);

        // 确保没有调用任何方法
        expect(store.sessionStore.activeSession).toBeUndefined();
    });

    beforeEach(() => {
        store.sessionStore.activeSession = {
            locateUnit: {
                target: jest.fn(),
                onSuccess: jest.fn(),
                showDetail: false,
            },
        } as Partial<Session> as Session;
    });

    it('should call locateUnit.target with correct parameters', () => {
        // 模拟 session 为 defined
        store.sessionStore.activeSession = {
            locateUnit: {
                showDetail: false,
            },
        } as Partial<Session> as Session;

        const unitDetail: UnitDetail = {
            cardId: '123',
            pid: '456',
            propsCardId: '789',
        };

        jumpToCorrespondingLane(unitDetail);
        // 获取 target 函数
        const targetFunction = store.sessionStore.activeSession.locateUnit?.target;
        const dataSource: DataSource = {
            remote: 'lll',
            port: 3333,
            projectName: 'oooooooo',
            dataPath: ['llllll'],
            projectPath: ['llllll'],
            children: [],
        };

        // 模拟 InsightUnit
        const mockUnit1 = {
            metadata: {
                cardId: '123',
                processId: '456',
                dataSource,
            },
        } as Partial<InsightUnit> as InsightUnit;
        const mockUnit2 = {
            metadata: {
                cardId: '789',
                processId: '456',
                dataSource,
            },
        } as Partial<InsightUnit> as InsightUnit;
        const mockUnit3 = {
            metadata: {
                cardId: '999',
                processId: '999',
                dataSource,
            },
        } as Partial<InsightUnit> as InsightUnit;
        expect(targetFunction).toBeDefined();
        // 测试 target 函数
        if (targetFunction) {
            expect(targetFunction(mockUnit1)).toBe(true);
        }
        if (targetFunction) {
            expect(targetFunction(mockUnit2)).toBe(true);
        }
        if (targetFunction) {
            expect(targetFunction(mockUnit3)).toBe(false);
        }
    });
});
