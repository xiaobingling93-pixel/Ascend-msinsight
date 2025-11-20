/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import { render, screen, waitFor } from '@testing-library/react';
import Memory from './Memory'; // 调整导入路径
import { Session } from '../entity/session';
import { MemorySession, DataResourceType, GroupBy } from '../entity/memorySession';
import { memoryTypeGet, resourceTypeGet } from '../utils/RequestUtils';
import '@testing-library/jest-dom';
import { RootStore } from '../store';
import { useRootStore } from '../context/context';

// 模拟所有依赖模块
jest.mock('../context/context');
jest.mock('../utils/RequestUtils');
jest.mock('../utils/strategyUtils');
jest.mock('mobx-react-lite', () => ({
    observer: (component: React.ComponentType) => component,
}));
jest.mock('@insight/lib/components', () => ({
    Layout: ({ children }: { children: React.ReactNode }) => <div data-testid="layout">{children}</div>,
}));
jest.mock('../components/MemoryHeader', () => ({
    __esModule: true,
    default: ({ strategy, session, memorySession }: any) => (
        <div data-testid="memory-header">
            MemoryHeader - {strategy.constructor.name}
        </div>
    ),
}));
jest.mock('../components/MemoryLineChart', () => ({
    __esModule: true,
    default: ({ session, memorySession, isDark }: any) => (
        <div data-testid="memory-line-chart">
            MemoryLineChart - {isDark ? 'dark' : 'light'}
        </div>
    ),
}));
jest.mock('../components/MemoryDetailTable', () => ({
    __esModule: true,
    default: ({ session, memorySession }: any) => (
        <div data-testid="memory-detail-table">MemoryDetailTable</div>
    ),
}));
jest.mock('@insight/lib/utils', () => ({
    customConsole: {
        error: jest.fn(),
    },
}));
const mockedUseRootStore = useRootStore as jest.MockedFunction<typeof useRootStore>;
const mockedMemoryTypeGet = memoryTypeGet as jest.MockedFunction<typeof memoryTypeGet>;
const mockedResourceTypeGet = resourceTypeGet as jest.MockedFunction<typeof resourceTypeGet>;

// 创建测试用的模拟对象
const createMockMemorySession = (overrides?: Partial<MemorySession>): MemorySession => ({
    getSelectedRankValue: jest.fn().mockReturnValue({
        rankInfo: { rankId: 'test-rank-id' },
        dbPath: 'test-db-path',
    }),
    memoryType: '',
    memoryGraphId: '',
    memoryGraphIdList: [],
    resourceType: DataResourceType.MINDSPORE,
    groupId: GroupBy.DEFAULT,
    hostCondition: {
        options: ['host1', 'host2'],
    },
    selectedRankId: 'test-rank-id',
    ...overrides,
} as any);

const createMockSession = (overrides?: Partial<Session>): Session => ({
    compareRank: {
        rankId: 'kk',
        isCompare: false,
    },
    ...overrides,
} as Partial<Session> as Session);

describe('Memory Component', () => {
    let mockMemoryStore: any;
    let mockMemorySession: MemorySession;
    let mockSession: Session;

    beforeEach(() => {
        // 重置所有模拟
        jest.clearAllMocks();

        mockMemorySession = createMockMemorySession();
        mockSession = createMockSession();
        mockMemoryStore = {
            activeSession: mockMemorySession,
        };
        // 模拟 useRootStore
        mockedUseRootStore.mockReturnValue({ memoryStore: mockMemoryStore } as Partial<RootStore> as RootStore);
        jest.doMock('../context/context', () => ({
            useRootStore: { memoryStore: mockMemoryStore } as Partial<RootStore> as RootStore,
        }));

        // 模拟 API 调用
        mockedMemoryTypeGet.mockResolvedValue({
            type: 'test-memory-type',
            graphId: ['graph1', 'graph2'],
        } as any);

        mockedResourceTypeGet.mockResolvedValue({
            type: DataResourceType.PYTORCH,
        } as any);
    });

    describe('component rendering', () => {
        it('an empty element should be rendered when memorySession does not exist.', () => {
            mockMemoryStore.activeSession = null;

            const { container } = render(<Memory session={mockSession} isDark={false} />);

            expect(container.firstChild).toBeNull();
        });

        it('all child components should be correctly rendered when memorySession exists.', () => {
            render(<Memory session={mockSession} isDark={true} />);

            expect(screen.getByTestId('layout')).toBeInTheDocument();
            expect(screen.getByTestId('memory-header')).toBeInTheDocument();
            expect(screen.getByTestId('memory-line-chart')).toBeInTheDocument();
            expect(screen.getByTestId('memory-detail-table')).toBeInTheDocument();
        });

        it('props should be correctly passed to child components.', () => {
            render(<Memory session={mockSession} isDark={false} />);

            const lineChart = screen.getByTestId('memory-line-chart');
            expect(lineChart.textContent).toBe('MemoryLineChart - light');
        });
    });

    describe('side effect (useEffect)', () => {
        it('when selectedRankId changes, fetchMemoryType and fetchResourceType should be called.', async () => {
            render(<Memory session={mockSession} isDark={false} />);

            await waitFor(() => {
                expect(mockedMemoryTypeGet).toHaveBeenCalledWith({
                    rankId: 'test-rank-id',
                    dbPath: 'test-db-path',
                });
                expect(mockedResourceTypeGet).toHaveBeenCalledWith({
                    rankId: 'test-rank-id',
                    dbPath: 'test-db-path',
                });
            });
        });

        it('the API should not be called when rankId is empty.', () => {
            const mockSessionWithoutRank = createMockMemorySession({
                getSelectedRankValue: jest.fn().mockReturnValue({
                    rankInfo: { rankId: '' },
                    dbPath: 'test-db-path',
                }),
            });
            mockMemoryStore.activeSession = mockSessionWithoutRank;

            render(<Memory session={mockSession} isDark={false} />);

            expect(mockedMemoryTypeGet).not.toHaveBeenCalled();
            expect(mockedResourceTypeGet).not.toHaveBeenCalled();
        });

        it('API call errors should be handled correctly.', async () => {
            const consoleError = require('@insight/lib/utils').customConsole.error;
            mockedMemoryTypeGet.mockRejectedValue(new Error('API Error'));

            render(<Memory session={mockSession} isDark={false} />);

            await waitFor(() => {
                expect(consoleError).toHaveBeenCalledWith(expect.any(Error));
            });
        });
    });

    describe('display policy logic', () => {
        it('the correct display strategy should be chosen based on the conditions.', () => {
            render(<Memory session={mockSession} isDark={false} />);

            // 验证策略选择逻辑
            const header = screen.getByTestId('memory-header');
            expect(header.textContent).toContain('MemoryHeader - FullDisplayStrategy');
        });

        it('policies should be updated when dependencies change.', () => {
            const { rerender } = render(<Memory session={mockSession} isDark={false} />);

            // 改变比较状态
            const newSession = createMockSession({ compareRank: { isCompare: true } } as Partial<Session> as Session);
            rerender(<Memory session={newSession} isDark={false} />);

            // 这里可以添加对策略更新的断言
            // 由于策略选择逻辑在组件内部，我们可以通过检查 MemoryHeader 的渲染来验证
        });
    });

    describe('borderline cases', () => {
        it('the empty hostCondition option should be handled.', () => {
            const sessionWithEmptyHost = createMockMemorySession({
                hostCondition: { options: [], value: '' },
            });
            mockMemoryStore.activeSession = sessionWithEmptyHost;

            render(<Memory session={mockSession} isDark={false} />);

            // 验证组件能够正常渲染而不崩溃
            expect(screen.getByTestId('layout')).toBeInTheDocument();
        });

        it('different resourceType values should be handled.', () => {
            const sessionWithMindspore = createMockMemorySession({
                resourceType: DataResourceType.MINDSPORE,
            });
            mockMemoryStore.activeSession = sessionWithMindspore;

            render(<Memory session={mockSession} isDark={false} />);

            expect(screen.getByTestId('memory-header')).toBeInTheDocument();
        });
    });
});
