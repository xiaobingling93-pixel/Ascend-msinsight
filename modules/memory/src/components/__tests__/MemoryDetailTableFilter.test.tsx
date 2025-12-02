/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
// __tests__/MemoryDetailTableFilter.test.tsx
import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { useTranslation } from 'react-i18next';
import MemoryDetailTableFilter from '../MemoryDetailTableFilter';
import { Session } from '../../entity/session';
import { MemorySession, MemoryGraphType, DEFAULT_SHOW_WITHIN_INTERVAL, GroupBy } from '../../entity/memorySession';
import { Label } from '../Common';
import '@testing-library/jest-dom';
import { Button, Input, InputNumber } from '@insight/lib/components';
import userEvent from '@testing-library/user-event';
import { memoryCurveGet } from '../../utils/RequestUtils';

// Mock dependencies
jest.mock('mobx-react-lite', () => ({
    observer: (component) => component,
}));

jest.mock('react-i18next', () => ({
    useTranslation: jest.fn(),
}));

jest.mock('@insight/lib/components', () => ({
    Button: ({ children, onClick, disabled, ...props }) => (
        <button onClick={onClick} disabled={disabled} {...props} data-testid={props['data-testid']}>
            {children}
        </button>
    ),
    Input: ({ value, onChange, placeholder, ...props }) => (
        <input
            data-testid="mock-input"
            value={value}
            onChange={onChange}
            placeholder={placeholder}
            {...props}
        />
    ),
    InputNumber: ({ value, onChange, ...props }) => (
        <input
            type="number"
            value={value}
            onChange={(e) => onChange?.(e.target.valueAsNumber)}
            {...props}
        />
    ),
}));

jest.mock('../../utils/styleUtils', () => ({
    SearchBox: ({ children }) => <div data-testid="search-box">{children}</div>,
}));

jest.mock('../Common', () => ({
    Label: ({ name }) => <label>{name}</label>,
}));

jest.mock('../OptionalCheckbox', () => {
    return function MockOptionalCheckbox({ visible, name, value, onChange, disabled, idKey }) {
        if (!visible) return null;

        return (
            <div>
                <label>
                    <input
                        type="checkbox"
                        checked={value}
                        onChange={(e) => onChange?.({ target: { checked: e.target.checked } })}
                        disabled={disabled}
                        data-testid={idKey}
                    />
                    {name}
                </label>
            </div>
        );
    };
});

// Mock data
const createMockSession = (isCompare = false) => ({
    compareRank: {
        isCompare,
    },
});

const createMockMemorySession = (overrides = {}) => ({
    memoryType: MemoryGraphType.STATIC,
    isBtnDisabled: false,
    searchEventOperatorName: '',
    minSize: 0,
    maxSize: 100,
    defaultMinSize: 0,
    defaultMaxSize: 100,
    isOnlyShowAllocatedOrReleasedWithinInterval: DEFAULT_SHOW_WITHIN_INTERVAL,
    ...overrides,
});

const mockQueryDetailData = jest.fn();
const mockOnReset = jest.fn();

jest.mock('react-i18next', () => ({
    useTranslation: () => ({
        t: (key) => {
            const translations = {
                'searchCriteria.Name': 'Name',
                'searchCriteria.Search by Name': 'Search by Name',
                'searchCriteria.Min Size': 'Min Size',
                'searchCriteria.Max Size': 'Max Size',
                'searchCriteria.Show Allocated or Released Within Interval Data': 'Show interval data',
                'searchCriteria.Button Query': 'Query',
                'searchCriteria.Button Reset': 'Reset',
            };
            return translations[key] || key;
        },
    }),
}));

beforeEach(() => {
    jest.clearAllMocks();
});

describe('MemoryDetailTableFilter', () => {
    describe('Static and Compare Mode', () => {
        it('renders all filter controls when memoryType is STATIC', () => {
            const session = createMockSession();
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.STATIC });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            expect(screen.getByPlaceholderText('Search by Name')).toBeInTheDocument();
            expect(screen.getByDisplayValue('0')).toBeInTheDocument(); // minSize
            expect(screen.getByDisplayValue('100')).toBeInTheDocument(); // maxSize
            expect(screen.getByText('Query')).toBeInTheDocument();
            expect(screen.getByText('Reset')).toBeInTheDocument();
        });

        it('renders all filter controls when isCompare is true (even with DYNAMIC memoryType)', () => {
            const session = createMockSession(true);
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.DYNAMIC });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            expect(screen.getByPlaceholderText('Search by Name')).toBeInTheDocument();
            expect(screen.getByText('Query')).toBeInTheDocument();
            expect(screen.getByText('Reset')).toBeInTheDocument();
        });

        it('handles name input change in static mode', async () => {
            const session = createMockSession();
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.STATIC });
            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );
            const nameInput = screen.getByPlaceholderText('Search by Name');
            fireEvent.change(nameInput, { target: { value: 'test operator' } });
            expect(memorySession.searchEventOperatorName).toBe('test operator');
        });

        it('handles min and max size input changes in static mode', () => {
            const session = createMockSession();
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.STATIC });
            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            const minSizeInput = screen.getByDisplayValue('0');
            const maxSizeInput = screen.getByDisplayValue('100');

            fireEvent.change(minSizeInput, { target: { value: 50 } });
            fireEvent.change(maxSizeInput, { target: { value: 200 } });

            expect(memorySession.maxSize).toBe(200);
            expect(memorySession.minSize).toBe(50);
        });

        it('handles query and reset button clicks in static mode', () => {
            const session = createMockSession();
            const memorySession = createMockMemorySession({
                memoryType: MemoryGraphType.STATIC,
                searchEventOperatorName: 'test',
                minSize: 50,
                maxSize: 150,
            });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            const queryButton = screen.getByTestId('query-btn');
            const resetButton = screen.getByTestId('reset-btn');

            fireEvent.click(queryButton);
            expect(mockQueryDetailData).toHaveBeenCalledWith(true);

            fireEvent.click(resetButton);
            expect(mockQueryDetailData).toHaveBeenCalledWith(false);
        });

        it('disables buttons when isBtnDisabled is true in static mode', () => {
            const session = createMockSession();
            const memorySession = createMockMemorySession({
                memoryType: MemoryGraphType.STATIC,
                isBtnDisabled: true,
            });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            const queryButton = screen.getByTestId('query-btn');
            const resetButton = screen.getByTestId('reset-btn');

            expect(queryButton).toBeDisabled();
            expect(resetButton).toBeDisabled();
        });

        it('hides checkbox when memoryType is STATIC', () => {
            const session = createMockSession();
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.STATIC });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            expect(screen.queryByTestId('input-onlyShowAllocatedOrReleased')).not.toBeInTheDocument();
        });

        it('shows and handles checkbox when memoryType is not STATIC but isCompare is true', () => {
            const session = createMockSession(true);
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.DYNAMIC });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            const checkbox = screen.getByTestId('input-onlyShowAllocatedOrReleased');
            expect(checkbox).toBeInTheDocument();

            fireEvent.click(checkbox);
            expect(memorySession.isOnlyShowAllocatedOrReleasedWithinInterval).toBe(true);
        });

        it('resets filters when isCompare changes in static mode', () => {
            const session = createMockSession(false);
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.STATIC });

            const { rerender } = render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            // Change isCompare to true
            const newSession = createMockSession(true);
            rerender(
                <MemoryDetailTableFilter
                    session={newSession as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            expect(mockQueryDetailData).toHaveBeenCalledWith(false);
        });
    });

    describe('Dynamic Mode', () => {
        it('renders only checkbox control when memoryType is DYNAMIC and isCompare is false', () => {
            const session = createMockSession(false);
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.DYNAMIC });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            expect(screen.getByTestId('input-onlyShowAllocatedOrReleased')).toBeInTheDocument();
            expect(screen.queryByPlaceholderText('Search by Name')).not.toBeInTheDocument();
            expect(screen.queryByText('Query')).not.toBeInTheDocument();
            expect(screen.queryByText('Reset')).toBeInTheDocument();
        });

        it('handles checkbox change and triggers query in dynamic mode', () => {
            const session = createMockSession(false);
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.DYNAMIC });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            const checkbox = screen.getByTestId('input-onlyShowAllocatedOrReleased');
            fireEvent.click(checkbox);

            expect(mockQueryDetailData).toHaveBeenCalledWith(true);
        });

        it('disables checkbox when isBtnDisabled is true in dynamic mode', () => {
            const session = createMockSession(false);
            const memorySession = createMockMemorySession({
                memoryType: MemoryGraphType.DYNAMIC,
                isBtnDisabled: true,
            });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            const checkbox = screen.getByTestId('input-onlyShowAllocatedOrReleased');
            expect(checkbox).toBeDisabled();
        });

        it('resets checkbox state when isCompare changes in dynamic mode', async () => {
            const session = createMockSession(false);
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.STATIC });

            const { rerender } = render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            // Change isCompare to true
            const newSession = createMockSession(true);
            rerender(
                <MemoryDetailTableFilter
                    session={newSession as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );
            // Should switch to static mode rendering, so checkbox should not be present
            expect(screen.queryByTestId('input-onlyShowAllocatedOrReleased')).not.toBeInTheDocument();
        });
    });

    describe('Edge Cases', () => {
        it('handles null or undefined values in number inputs', () => {
            const session = createMockSession();
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.STATIC });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            const minSizeInput = screen.getByDisplayValue('0');
            fireEvent.change(minSizeInput, { target: { value: null } });

            // The component should handle null values gracefully
            expect(memorySession.minSize).toBe(NaN);
        });

        it('handles extremely large number inputs', () => {
            const session = createMockSession();
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.STATIC });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            const maxSizeInput = screen.getByDisplayValue('100');
            fireEvent.change(maxSizeInput, { target: { value: 4294967295 } }); // MAX_INPUT_NUMBER
            expect(memorySession.maxSize).toBe(4294967295);
        });

        it('handles negative numbers in compare mode', () => {
            const session = createMockSession(true);
            const memorySession = createMockMemorySession({ memoryType: MemoryGraphType.STATIC });

            render(
                <MemoryDetailTableFilter
                    session={session as Session}
                    memorySession={memorySession as MemorySession}
                    queryDetailData={mockQueryDetailData}
                    onReset={mockOnReset}
                />,
            );

            const minSizeInput = screen.getByDisplayValue('0');
            fireEvent.change(minSizeInput, { target: { value: -100 } });
            expect(memorySession.minSize).toBe(-100);
        });
    });
});
