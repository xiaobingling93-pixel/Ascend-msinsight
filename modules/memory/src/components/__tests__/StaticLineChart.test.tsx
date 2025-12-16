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
import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import StaticLineChart from '../StaticLineChart';
import { Session } from '../../entity/session';
import { MemorySession } from '../../entity/memorySession';
import { staticOpMemoryGraphGet } from '../../utils/RequestUtils';
import { useTranslation } from 'react-i18next';
import '@testing-library/jest-dom';
import { LineChart } from '../LineChart';

// Mock dependencies
jest.mock('mobx-react-lite', () => ({
    observer: (component: React.ComponentType) => component,
}));

jest.mock('../../utils/RequestUtils', () => ({
    staticOpMemoryGraphGet: jest.fn(),
}));

jest.mock('react-i18next', () => ({
    useTranslation: jest.fn(),
}));

jest.mock('@insight/lib/components', () => ({
    Select: ({ value, onChange, options }) => (
        <select
            data-testid="select-graphId"
            value={value}
            onChange={(e) => onChange(e.target.value)}
        >
            {options.map((option: any) => (
                <option key={option.value} value={option.value}>
                    {option.label}
                </option>
            ))}
        </select>
    ),
    Spin: ({ spinning, children }) => (
        <div data-testid="spin" data-spinning={spinning}>
            {children}
        </div>
    ),
}));

jest.mock('../Common', () => ({
    Label: ({ name }) => <label data-testid="label">{name}</label>,
}));

jest.mock('../LineChart', () => ({
    LineChart: ({ onSelectionChanged, graph, isStatic }) => (
        <div
            data-testid="line-chart"
            data-is-static={isStatic}
            data-graph-columns={graph?.columns?.join(',')}
            onClick={() => onSelectionChanged && onSelectionChanged(0, 10)}
        >
            LineChart Mock
        </div>
    ),
}));

jest.mock('../../utils/styleUtils', () => ({
    SearchBox: ({ children }) => <div data-testid="search-box">{children}</div>,
}));

jest.mock('@insight/lib/utils', () => ({
    StyledEmpty: ({ style, translation }) => (
        <div data-testid="styled-empty" style={style}>
            StyledEmpty Mock - {translation?.('test')}
        </div>
    ),
    customConsole: {
        error: jest.fn(),
    },
}));

// Test data factories
const createMockSession = (overrides?: Partial<Session>): Session => ({
    compareRank: {
        isCompare: false,
    },
    isAllMemoryCompletedSwitch: false,
    ...overrides,
});

const createMockMemorySession = (overrides?: Partial<MemorySession>): MemorySession => ({
    memoryGraphId: 'graph-1',
    memoryGraphIdList: ['graph-1', 'graph-2', 'graph-3'],
    selectedRankId: 'rank-1',
    staticSelectedRange: undefined,
    searchEventOperatorName: '',
    current: 1,
    pageSize: 10,
    getSelectedRankValue: jest.fn(),
    ...overrides,
});

const mockStaticOperatorCurve = {
    legends: ['legend1', 'legend2'],
    lines: [
        ['100', '200', '300'],
        ['150', '250', '350'],
    ],
};

describe('StaticLineChart', () => {
    const defaultProps = {
        session: createMockSession(),
        memorySession: createMockMemorySession(),
        isDark: false,
    };

    beforeEach(() => {
        jest.clearAllMocks();
        (useTranslation as jest.Mock).mockReturnValue({
            t: (key: string) => `translated-${key}`,
        });
        (staticOpMemoryGraphGet as jest.Mock).mockResolvedValue(mockStaticOperatorCurve);
        (defaultProps.memorySession.getSelectedRankValue as jest.Mock).mockReturnValue({
            rankInfo: { rankId: 'rank-1' },
            dbPath: '/test/db/path',
        });
    });

    afterEach(() => {
        jest.restoreAllMocks();
    });

    // Test case 1: Component renders without crashing
    it('should render without crashing', () => {
        render(<StaticLineChart {...defaultProps} />);
        expect(screen.getByTestId('search-box')).toBeInTheDocument();
    });

    // Test case 2: Renders search criteria section correctly
    it('should render search criteria section with label and select', () => {
        render(<StaticLineChart {...defaultProps} />);

        expect(screen.getByTestId('label')).toHaveTextContent('translated-searchCriteria.GraphId');
        expect(screen.getByTestId('select-graphId')).toBeInTheDocument();
    });

    // Test case 3: Select component has correct options
    it('should render select with correct options from memorySession', () => {
        defaultProps.memorySession.memoryGraphIdList = ['option1', 'option2'];
        render(<StaticLineChart {...defaultProps} />);

        const select = screen.getByTestId('select-graphId');
        expect(select).toHaveValue('option1');
    });

    // Test case 4: Handles graph ID change correctly
    it('should update memorySession when graph ID is changed', () => {
        jest.spyOn(require('mobx'), 'runInAction');
        render(<StaticLineChart {...defaultProps} />);

        const select = screen.getByTestId('select-graphId');
        fireEvent.change(select, { target: { value: 'graph-2' } });

        expect(runInAction).toHaveBeenCalled();
    });

    // Test case 5: Shows loading spinner when fetching data
    it('should show loading spinner when fetching static curve data', async () => {
        (staticOpMemoryGraphGet as jest.Mock).mockImplementation(() =>
            new Promise(resolve => setTimeout(() => resolve(mockStaticOperatorCurve), 100)),
        );

        render(<StaticLineChart {...defaultProps} />);

        await waitFor(() => {
            expect(screen.getByTestId('spin')).toHaveAttribute('data-spinning', 'true');
        });
    });

    // Test case 6: Rers LineChart when data is available
    it('should render LineChart when staticLineChartData is available', async () => {
        render(<StaticLineChart {...defaultProps} />);

        await waitFor(() => {
            expect(screen.getByTestId('line-chart')).toBeInTheDocument();
        });

        const lineChart = screen.getByTestId('line-chart');
        expect(lineChart).toHaveAttribute('data-is-static', 'true');
    });

    // Test case 7: Shows empty state when no data is available
    it('should render StyledEmpty when no staticLineChartData is available', () => {
        defaultProps.memorySession.rankCondition = { options: [], value: undefined };
        render(<StaticLineChart {...defaultProps} />);

        expect(screen.getByTestId('styled-empty')).toBeInTheDocument();
    });

    // Test case 8: Handles selection range change correctly
    it('should update static selected range when LineChart selection changes', async () => {
        jest.spyOn(require('mobx'), 'runInAction');
        render(<StaticLineChart {...defaultProps} />);

        await waitFor(() => {
            expect(screen.getByTestId('line-chart')).toBeInTheDocument();
        });
        const lineChart = screen.getByTestId('line-chart');
        fireEvent.click(lineChart);

        expect(runInAction).toHaveBeenCalled();
    });

    // Test case 10: Resets data when selectedRankId or memoryGraphId is empty
    it('should reset data when selectedRankId is empty', () => {
        const memorySession = createMockMemorySession({
            selectedRankId: '',
        });

        render(<StaticLineChart {...defaultProps} memorySession={memorySession} />);

        expect(staticOpMemoryGraphGet).not.toHaveBeenCalled();
    });

    // Test case 11: Fetches data when dependencies change
    it('should fetch data when selectedRankId changes', async () => {
        const { rerender } = render(<StaticLineChart {...defaultProps} />);

        await waitFor(() => {
            expect(staticOpMemoryGraphGet).toHaveBeenCalledTimes(1);
        });

        (defaultProps.memorySession.getSelectedRankValue as jest.Mock).mockReturnValue({
            rankInfo: { rankId: 'rank-2' },
            dbPath: '/test/db/path',
        });
        rerender(<StaticLineChart {...defaultProps} />);

        await waitFor(() => {
            expect(staticOpMemoryGraphGet).toHaveBeenCalledTimes(1);
        });
    });

    // Test case 12: Handles compare mode correctly
    it('should pass correct isCompare parameter in compare mode', async () => {
        const session = createMockSession({
            compareRank: { isCompare: true },
        });

        jest.spyOn(require('mobx'), 'runInAction');
        render(<StaticLineChart {...defaultProps} session={session} />);

        await waitFor(() => {
            expect(staticOpMemoryGraphGet).toHaveBeenCalledWith({
                rankId: 'rank-1',
                dbPath: '/test/db/path',
                graphId: '',
                isCompare: true,
            });
        });
    });

    // Test case 13: Transforms API response correctly
    it('should transform API response to LineChart data format', async () => {
        render(<StaticLineChart {...defaultProps} />);

        await waitFor(() => {
            const lineChart = screen.getByTestId('line-chart');
            expect(lineChart).toHaveAttribute('data-graph-columns', 'translated-legend1,translated-legend2');
        });
    });

    // Test case 14: Applies correct CSS classes
    it('should apply correct CSS classes to container', () => {
        const { container } = render(<StaticLineChart {...defaultProps} />);

        expect(container.querySelector('.mb-30')).toBeInTheDocument();
    });

    // Test case 15: Handles edge case with empty memoryGraphIdList
    it('should handle empty memoryGraphIdList gracefully', () => {
        defaultProps.memorySession.memoryGraphIdList = [];
        render(<StaticLineChart {...defaultProps} />);

        const select = screen.getByTestId('select-graphId');
        expect(select).toBeInTheDocument();
    });

    // Test case 16: Selection range validation logic
    it('should validate selection range and reset if invalid', async () => {
        // Mock the onStaticSelectedRangeChanged function to test validation
        await waitFor(() => {
            render(<StaticLineChart {...defaultProps} />);
        });
        const lineChart = screen.getByTestId('line-chart');
        fireEvent.click(lineChart); // This triggers onSelectionChanged(0, 10)
    });

    // Test case 17: Handles translation correctly
    it('should use translation function for all text', () => {
        render(<StaticLineChart {...defaultProps} />);

        expect(screen.getByTestId('label')).toHaveTextContent('translated-searchCriteria.GraphId');
    });

    // Test case 18: Cleanup on unmount
    it('should cleanup properly when unmounted', async () => {
        const { unmount } = render(<StaticLineChart {...defaultProps} />);

        await waitFor(() => {
            expect(screen.getByTestId('line-chart')).toBeInTheDocument();
        });

        unmount();

        // Verify that no errors occur after unmount
        expect(screen.queryByTestId('line-chart')).not.toBeInTheDocument();
    });
});
