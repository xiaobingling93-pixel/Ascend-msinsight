/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import { render, screen } from '@testing-library/react';
import { observer } from 'mobx-react-lite';
import MemoryLineChart from '../MemoryLineChart';
import { Session } from '../../entity/session';
import { MemorySession, MemoryGraphType } from '../../entity/memorySession';
import DynamicLineChart from '../DynamicLineChart';
import StaticLineChart from '../StaticLineChart';
import '@testing-library/jest-dom';

// Mock the child components
jest.mock('../DynamicLineChart', () => {
    return function MockDynamicLineChart(props: any) {
        return <div data-testid="dynamic-line-chart" data-props={JSON.stringify(props)}>DynamicLineChart</div>;
    };
});

jest.mock('../StaticLineChart', () => {
    return function MockStaticLineChart(props: any) {
        return <div data-testid="static-line-chart" data-props={JSON.stringify(props)}>StaticLineChart</div>;
    };
});

// Mock mobx observer
jest.mock('mobx-react-lite', () => ({
    observer: (component: React.ComponentType) => component,
}));

// Test data factories
const createMockSession = (overrides?: Partial<Session>): Session => ({
    // Add required Session properties here
    ...overrides,
});

const createMockMemorySession = (overrides?: Partial<MemorySession>): MemorySession => ({
    memoryType: MemoryGraphType.DYNAMIC,
    // Add other required MemorySession properties here
    ...overrides,
});

describe('MemoryLineChart', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('should render without crashing', () => {
        const session = createMockSession();
        const memorySession = createMockMemorySession();

        render(
            <MemoryLineChart
                session={session}
                memorySession={memorySession}
                isDark={false}
            />,
        );

        expect(screen.getByTestId('dynamic-line-chart')).toBeInTheDocument();
    });

    it('should render only DynamicLineChart when memoryType is DYNAMIC', () => {
        const session = createMockSession();
        const memorySession = createMockMemorySession({
            memoryType: MemoryGraphType.DYNAMIC,
        });

        render(
            <MemoryLineChart
                session={session}
                memorySession={memorySession}
                isDark={false}
            />,
        );

        expect(screen.getByTestId('dynamic-line-chart')).toBeInTheDocument();
        expect(screen.queryByTestId('static-line-chart')).not.toBeInTheDocument();
    });

    it('should render both DynamicLineChart and StaticLineChart when memoryType is STATIC', () => {
        const session = createMockSession();
        const memorySession = createMockMemorySession({
            memoryType: MemoryGraphType.STATIC,
        });

        render(
            <MemoryLineChart
                session={session}
                memorySession={memorySession}
                isDark={true}
            />,
        );

        expect(screen.getByTestId('dynamic-line-chart')).toBeInTheDocument();
        expect(screen.getByTestId('static-line-chart')).toBeInTheDocument();

        // Check that both charts are wrapped in a div
        const chartsContainer = screen.getByTestId('dynamic-line-chart').parentElement;
        expect(chartsContainer).toContainElement(screen.getByTestId('static-line-chart'));
    });

    it('should render DynamicLineChart as default when memoryType is unknown', () => {
        const session = createMockSession();
        const memorySession = createMockMemorySession({
            memoryType: 'UNKNOWN_TYPE' as MemoryGraphType, // Force unknown type
        });

        render(
            <MemoryLineChart
                session={session}
                memorySession={memorySession}
                isDark={false}
            />,
        );

        expect(screen.getByTestId('dynamic-line-chart')).toBeInTheDocument();
        expect(screen.queryByTestId('static-line-chart')).not.toBeInTheDocument();
    });

    it('should pass correct props to DynamicLineChart', () => {
        const session = createMockSession({ id: 'test-session' });
        const memorySession = createMockMemorySession();
        const isDark = true;

        render(
            <MemoryLineChart
                session={session}
                memorySession={memorySession}
                isDark={isDark}
            />,
        );

        const dynamicChart = screen.getByTestId('dynamic-line-chart');
        const props = JSON.parse(dynamicChart.getAttribute('data-props') || '{}');

        expect(props.session).toEqual(session);
        expect(props.memorySession).toEqual(memorySession);
        expect(props.isDark).toBe(isDark);
    });

    it('should pass correct props to StaticLineChart when rendered', () => {
        const session = createMockSession({ id: 'test-session' });
        const memorySession = createMockMemorySession({
            memoryType: MemoryGraphType.STATIC,
        });
        const isDark = false;

        render(
            <MemoryLineChart
                session={session}
                memorySession={memorySession}
                isDark={isDark}
            />,
        );

        const staticChart = screen.getByTestId('static-line-chart');
        const props = JSON.parse(staticChart.getAttribute('data-props') || '{}');

        expect(props.session).toEqual(session);
        expect(props.memorySession).toEqual(memorySession);
        expect(props.isDark).toBe(isDark);
    });

    it('should have correct CSS class on container', () => {
        const session = createMockSession();
        const memorySession = createMockMemorySession();

        const { container } = render(
            <MemoryLineChart
                session={session}
                memorySession={memorySession}
                isDark={false}
            />,
        );

        // Check that the container has the mb-30 class
        expect(container.firstChild).toHaveClass('mb-30');
    });

    it('should render DynamicLineChart for undefined memoryType', () => {
        const session = createMockSession();
        const memorySession = createMockMemorySession({
            memoryType: undefined as unknown as MemoryGraphType, // Force undefined type
        });

        render(
            <MemoryLineChart
                session={session}
                memorySession={memorySession}
                isDark={false}
            />,
        );

        expect(screen.getByTestId('dynamic-line-chart')).toBeInTheDocument();
        expect(screen.queryByTestId('static-line-chart')).not.toBeInTheDocument();
    });

    describe('MemoryGraphType enum coverage', () => {
        it('should handle all known MemoryGraphType values', () => {
            const session = createMockSession();

            // Test each known enum value
            Object.values(MemoryGraphType).forEach(memoryType => {
                const memorySession = createMockMemorySession({ memoryType });

                const { unmount } = render(
                    <MemoryLineChart
                        session={session}
                        memorySession={memorySession}
                        isDark={false}
                    />,
                );

                // Should always render without error
                expect(screen.getByTestId('dynamic-line-chart')).toBeInTheDocument();

                if (memoryType === MemoryGraphType.STATIC) {
                    expect(screen.getByTestId('static-line-chart')).toBeInTheDocument();
                } else {
                    expect(screen.queryByTestId('static-line-chart')).not.toBeInTheDocument();
                }

                unmount();
            });
        });
    });

    it('should maintain component structure with proper nesting', () => {
        const session = createMockSession();
        const memorySession = createMockMemorySession({
            memoryType: MemoryGraphType.STATIC,
        });

        const { container } = render(
            <MemoryLineChart
                session={session}
                memorySession={memorySession}
                isDark={false}
            />,
        );

        // Check the overall structure
        const mainDiv = container.firstChild as HTMLElement;
        expect(mainDiv).toBeInTheDocument();
        expect(mainDiv.tagName).toBe('DIV');
        expect(mainDiv).toHaveClass('mb-30');

        // For STATIC type, there should be a div containing both charts
        const innerDiv = mainDiv.firstChild as HTMLElement;
        expect(innerDiv.tagName).toBe('DIV');
        expect(innerDiv.children).toHaveLength(2);
    });
});
