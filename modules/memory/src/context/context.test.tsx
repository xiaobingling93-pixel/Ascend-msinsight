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
import { render } from '@testing-library/react';
import React from 'react';
import { RootStoreContext, useRootStore } from './context';
import { Session } from '../entity/session';
import { SessionStore } from '../store/sessionStore';

// 简单的组件测试
describe('useRootStore Hook', () => {
    it('should throw error when used outside provider', () => {
        // 创建一个测试组件来捕获错误
        const TestComponent = (): React.ReactElement => {
            try {
                useRootStore();
                return <div>No Error</div>;
            } catch (error) {
                return <div>{(error as Error).message}</div>;
            }
        };

        const { container } = render(<TestComponent />);
        expect(container.textContent).toBe('RootStoreContext is undefined');
    });

    it('should return store when inside provider', () => {
        const mockSession = {
            unitcount: 3,
        } as Partial<Session> as Session;
        const mockSessionStore = {
            activeSession: mockSession,
        } as Partial<SessionStore> as SessionStore;
        const mockStore = { sessionStore: mockSessionStore } as any;

        const TestComponent = (): React.ReactElement => {
            const store = useRootStore();
            return <div>{store.sessionStore.activeSession?.unitcount}</div>;
        };

        const { container } = render(
            <RootStoreContext.Provider value={mockStore}>
                <TestComponent />
            </RootStoreContext.Provider>,
        );

        expect(container.textContent).toBe('3');
    });
});
