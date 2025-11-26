/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import { render, screen, fireEvent, waitFor, act } from '@testing-library/react';
import MemoryDetailTable from '../MemoryDetailTable';
import { Session } from '../../entity/session';
import { MemorySession, MemoryGraphType, GroupBy } from '../../entity/memorySession';
import { message } from 'antd';
import '@testing-library/jest-dom';
import {
    fetchDynamicOperatorMaxMin,
    fetchStaticOperatorMaxMin, memoryCurveGet,
    operatorsMemoryGet,
    staticOpMemoryListGet,
} from '../../utils/RequestUtils';
import { AntTableChart, TableByComponent } from './AntTableChart';

// Mock dependencies more comprehensively
jest.mock('antd', () => ({
    ...jest.requireActual('antd'),
    message: {
        warning: jest.fn(),
    },
}));
jest.mock('@insight/lib/components', () => ({
    Spin: ({ children, spinning }) => (
        <div data-testid="spin" data-spinning={spinning}>
            {children}
        </div>
    ),
    CollapsiblePanel: (props: any) => (
        <div data-testid="collapsible-panel">
            <div>{props.title}</div>
            {props.children}
        </div>
    ),
}));

jest.mock('react-i18next', () => ({
    useTranslation: () => ({
        t: (key: string) => {
            const translations: { [key: string]: string } = {
                'Memory Allocation/Release Details': 'Memory Allocation/Release Details',
                'Invalid Size Warning': 'Invalid size range',
                Difference: 'Difference',
                Baseline: 'Baseline',
                Comparison: 'Comparison',
            };
            return translations[key] || key;
        },
    }),
}));

jest.mock('mobx-react-lite', () => ({
    observer: (component: any) => component,
}));

jest.mock('mobx', () => ({
    runInAction: jest.fn((fn) => fn()),
}));

jest.mock('../../utils/RequestUtils', () => ({
    fetchDynamicOperatorMaxMin: jest.fn(),
    fetchStaticOperatorMaxMin: jest.fn(),
    operatorsMemoryGet: jest.fn(),
    staticOpMemoryListGet: jest.fn(),
}));

// Mock child components
jest.mock('../MemoryDetailTableFilter', () => ({
    __esModule: true,
    default: ({ session, memorySession, queryDetailData }: any) => (
        <div data-testid="memory-detail-table-filter">
            Filter Component
        </div>
    ),
}));

jest.mock('../AntTableChart', () => ({
    __esModule: true,
    AntTableChart: ({
        tableData,
        onRowSelected,
        needUseKeyMap,
        current,
        pageSize,
        onPageChange,
        onOrderChange,
        onOrderByChange,
        onFiltersChange,
        total,
        isCompare,
        selectedCard,
    }: any) => (
        <div data-testid="ant-table-chart" onClick={() => onPageChange && onPageChange(2, 20)}>
            Table Chart Component
        </div>
    ),
    TableByComponent: ({ session }: any) => (
        <div data-testid="table-by-component">
            Table By Component
        </div>
    ),
}));

// Mock console
const mockConsoleError = jest.fn();
jest.mock('@insight/lib/utils', () => ({
    customConsole: {
        error: (...args: any[]) => mockConsoleError(...args),
    },
}));

describe('MemoryDetailTable', () => {
    let mockSession: Session;
    let mockMemorySession: MemorySession;

    beforeEach(() => {
        // Reset all mocks
        jest.clearAllMocks();

        mockSession = {
            compareRank: {
                isCompare: false,
            },
            isAllMemoryCompletedSwitch: false,
        } as any;

        mockMemorySession = {
            memoryType: MemoryGraphType.DYNAMIC,
            selectedRankId: 'test-rank-id',
            groupId: GroupBy.DEFAULT,
            memoryGraphId: 'test-graph-id',
            current: 1,
            pageSize: 10,
            minSize: 0,
            maxSize: 100,
            defaultMinSize: 0,
            defaultMaxSize: 100,
            searchEventOperatorName: '',
            filters: {},
            rangeFilters: {},
            isOnlyShowAllocatedOrReleasedWithinInterval: false,
            isBtnDisabled: false,
            order: null as any,
            orderBy: undefined,
            selectedRecord: null,
            selectedStaticRecord: null,
            selectedRange: null,
            staticSelectedRange: null,
            getSelectedRankValue: jest.fn().mockReturnValue({
                rankInfo: { rankId: 'test-rank-id' },
                dbPath: 'test-db-path',
                index: 0,
            }),
        } as any;

        (fetchDynamicOperatorMaxMin as jest.Mock).mockResolvedValue({ minSize: 0, maxSize: 100 });
        (operatorsMemoryGet as jest.Mock).mockResolvedValue({
            operatorDetail: [],
            totalNum: 0,
            columnAttr: [],
        });
    });

    describe('Component Rendering', () => {
        it('renders without crashing', () => {
            render(<MemoryDetailTable session={mockSession} memorySession={mockMemorySession} />);
            expect(screen.getByText('Memory Allocation/Release Details')).toBeInTheDocument();
        });

        it('renders TableByComponent when groupId is COMPONENT', () => {
            const componentMemorySession = {
                ...mockMemorySession,
                groupId: GroupBy.COMPONENT,
            };

            render(<MemoryDetailTable session={mockSession} memorySession={componentMemorySession} />);
            expect(screen.getByTestId('table-by-component')).toBeInTheDocument();
        });

        it('renders MemoryDetailTableFilter and AntTableChart when groupId is OPERATOR', async () => {
            render(<MemoryDetailTable session={mockSession} memorySession={mockMemorySession} />);
            expect(screen.getByTestId('memory-detail-table-filter')).toBeInTheDocument();
            await waitFor(() => {
                expect(screen.getByTestId('ant-table-chart')).toBeInTheDocument();
            });
        });
    });

    describe('Data Fetching', () => {
        it('fetches size data and table data when component mounts with valid parameters', async () => {
            const mockResponse = {
                operatorDetail: [{ compare: { name: 'test', size: 100 } }],
                totalNum: 1,
                columnAttr: [{ title: 'Name', dataIndex: 'name' }],
            };

            (operatorsMemoryGet as jest.Mock).mockResolvedValue(mockResponse);
            (fetchDynamicOperatorMaxMin as jest.Mock).mockResolvedValue({ minSize: 0, maxSize: 100 });

            render(<MemoryDetailTable session={mockSession} memorySession={mockMemorySession} />);

            await waitFor(() => {
                expect(fetchDynamicOperatorMaxMin).toHaveBeenCalledWith({
                    rankId: 'test-rank-id',
                    dbPath: 'test-db-path',
                    type: 'Overall',
                    isCompare: false,
                });
            });

            await waitFor(() => {
                expect(operatorsMemoryGet).toHaveBeenCalledWith({
                    rankId: 'test-rank-id',
                    dbPath: 'test-db-path',
                    type: 'Overall',
                    currentPage: 1,
                    pageSize: 10,
                    searchName: '',
                    minSize: 0,
                    maxSize: 100,
                    filters: {},
                    rangeFilters: {},
                    isOnlyShowAllocatedOrReleasedWithinInterval: false,
                    isCompare: false,
                });
            });
        });

        it('handles API errors gracefully', async () => {
            (fetchDynamicOperatorMaxMin as jest.Mock).mockRejectedValue(new Error('API Error'));
            render(<MemoryDetailTable session={mockSession} memorySession={mockMemorySession} />);

            await waitFor(() => {
                expect(mockConsoleError).toHaveBeenCalled();
            });
        });
    });

    describe('Parameter Validation', () => {
        it('shows warning when maxSize is less than minSize', async () => {
            const invalidMemorySession = {
                ...mockMemorySession,
                minSize: 100,
                maxSize: 50,
            };

            await act(async () => {
                render(<MemoryDetailTable session={mockSession} memorySession={invalidMemorySession} />);
            });

            await waitFor(() => {
                expect(message.warning).toHaveBeenCalledWith('Invalid size range');
            });
        });

        it('does not fetch table data when groupId is COMPONENT', async () => {
            const componentMemorySession = {
                ...mockMemorySession,
                groupId: GroupBy.COMPONENT,
            };

            await act(async () => {
                render(<MemoryDetailTable session={mockSession} memorySession={componentMemorySession} />);
            });

            // Should not call table data API for component group
            expect(operatorsMemoryGet).not.toHaveBeenCalled();
        });
    });

    describe('Comparison Mode', () => {
        it('handles comparison data correctly when isCompare is true', async () => {
            const compareSession = {
                ...mockSession,
                compareRank: { isCompare: true },
            };

            const mockOperatorDetail = [
                {
                    diff: { name: 'diff-op', size: 100, source: 'Difference' },
                    baseline: { name: 'base-op', size: 50 },
                    compare: { name: 'comp-op', size: 150 },
                },
            ];

            (operatorsMemoryGet as jest.Mock).mockResolvedValue({
                operatorDetail: mockOperatorDetail,
                totalNum: 1,
                columnAttr: [{ title: 'Name', dataIndex: 'name' }],
            });
            await waitFor(async () => {
                render(<MemoryDetailTable session={compareSession} memorySession={mockMemorySession} />);
            });

            await waitFor(() => {
                expect(operatorsMemoryGet).toHaveBeenCalledWith(
                    expect.objectContaining({ isCompare: true }),
                );
            });
        });
    });

    describe('Memory Type Handling', () => {
        it('uses correct APIs for dynamic memory type', async () => {
            await waitFor(async () => {
                render(<MemoryDetailTable session={mockSession} memorySession={mockMemorySession} />);
            });

            await waitFor(() => {
                expect(fetchDynamicOperatorMaxMin).toHaveBeenCalled();
                expect(operatorsMemoryGet).toHaveBeenCalled();
            });
        });

        it('uses correct APIs for static memory type', async () => {
            const staticMemorySession = {
                ...mockMemorySession,
                memoryType: MemoryGraphType.STATIC,
            };

            (fetchStaticOperatorMaxMin as jest.Mock).mockResolvedValue({ minSize: 0, maxSize: 100 });
            (staticOpMemoryListGet as jest.Mock).mockResolvedValue({
                operatorDetail: [],
                totalNum: 0,
                columnAttr: [],
            });

            await waitFor(async () => {
                render(<MemoryDetailTable session={mockSession} memorySession={staticMemorySession} />);
            });

            await waitFor(() => {
                expect(fetchStaticOperatorMaxMin).toHaveBeenCalled();
                expect(staticOpMemoryListGet).toHaveBeenCalled();
            });
        });
    });

    describe('Edge Cases', () => {
        it('handles empty response data correctly', async () => {
            (operatorsMemoryGet as jest.Mock).mockResolvedValue({
                operatorDetail: [],
                totalNum: 0,
                columnAttr: [],
            });

            await waitFor(async () => {
                render(<MemoryDetailTable session={mockSession} memorySession={mockMemorySession} />);
            });

            await waitFor(() => {
                expect(screen.getByTestId('ant-table-chart')).toBeInTheDocument();
            });
        });
    });
});
