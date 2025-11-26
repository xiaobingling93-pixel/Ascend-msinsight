/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import { fireEvent, render, screen, waitFor } from '@testing-library/react';
import MemoryHeader from '../MemoryHeader';
import { MemoryHeaderStrategy } from '../../utils/strategyUtils';
import { CardRankInfo, Session } from '../../entity/session';
import { GroupBy, MemorySession } from '../../entity/memorySession';
import { getRankInfoLabel, GroupCardRankInfosByHost, notNull, transformCardIdInfo } from '@insight/lib/utils';
import '@testing-library/jest-dom';

// Mock dependencies
jest.mock('mobx-react-lite', () => ({
    observer: (component: React.ComponentType) => component,
}));

jest.mock('react-i18next', () => ({
    useTranslation: () => ({
        t: (key: string) => key,
    }),
}));

jest.mock('../Common', () => ({
    useHit: () => 'hit-indicator',
    Label: ({ name }: { name: string }) => <label>{name}</label>,
}));

jest.mock('@insight/lib/components', () => ({
    Select: ({ value, onChange, options, disabled, id }: any) => (
        <select
            id={id}
            value={value}
            onChange={(e) => onChange(e.target.value)}
            disabled={disabled}
            data-testid={id}
        >
            {options.map((option: any) => (
                <option key={option.value} value={option.value}>
                    {option.label}
                </option>
            ))}
        </select>
    ),
}));

jest.mock('@insight/lib/utils');

// Mock lodash cloneDeep
jest.mock('lodash', () => ({
    cloneDeep: jest.fn((obj) => JSON.parse(JSON.stringify(obj))),
}));

// Test data factories
const createMockSession = {
    memoryCardInfos: [],
    compareRank: {
        isCompare: false,
        rankId: 'host1-rank1',
    },
    isAllMemoryCompletedSwitch: false,
} as Partial<Session> as Session;

const createMockMemorySession = {
    hostCondition: {
        value: '',
        options: ['host1', 'host2'],
        cardsMap: new Map(),
    },
    rankCondition: {
        value: 0,
        options: [],
    },
    groupId: GroupBy.DEFAULT,
    selectedRankId: '',
} as Partial<MemorySession> as MemorySession;

const createMockStrategy = (shouldDisplay = true): MemoryHeaderStrategy => ({
    shouldDisplay: jest.fn(() => shouldDisplay),
});

describe('MemoryHeader', () => {
    beforeEach(() => {
        jest.clearAllMocks();
        (GroupCardRankInfosByHost as jest.Mock).mockReturnValue({
            hosts: ['host1', 'host2'],
            cardsMap: new Map([
                ['host1', [{ index: 0, rankInfo: { rankId: 'rank1' } }]],
                ['host2', [{ index: 0, rankInfo: { rankId: 'rank2' } }]],
            ]),
        });
        (transformCardIdInfo as jest.Mock).mockImplementation((rankId) => ({
            host: rankId.includes('host') ? rankId.split('-')[0] : '',
        }));
        (notNull as jest.Mock).mockImplementation((value) => value != null && value !== '');
        (getRankInfoLabel as jest.Mock).mockImplementation((rankInfo) => `label-${rankInfo.rankId}`);
    });

    it('should render without crashing', () => {
        const session = createMockSession;
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy();

        render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        expect(screen.getByTestId('select-groupId')).toBeInTheDocument();
    });

    it('should display all fields when strategy allows', () => {
        const session = createMockSession;
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy(true);

        render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        expect(screen.getByTestId('select-host')).toBeInTheDocument();
        expect(screen.getByTestId('select-rankId')).toBeInTheDocument();
        expect(screen.getByTestId('select-groupId')).toBeInTheDocument();
    });

    it('should hide fields when strategy returns false', () => {
        const session = createMockSession;
        const memorySession = createMockMemorySession;
        const strategy = {
            shouldDisplay: jest.fn((field) => field !== 'host'),
        };

        render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        expect(screen.queryByTestId('select-host')).not.toBeInTheDocument();
        expect(screen.getByTestId('select-rankId')).toBeInTheDocument();
        expect(screen.getByTestId('select-groupId')).toBeInTheDocument();
    });

    it('should handle host change correctly', () => {
        const session = createMockSession;
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy(true);

        const { getByTestId } = render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        const hostSelect = getByTestId('select-host');
        fireEvent.change(hostSelect, { target: { value: 'host2' } });

        expect(memorySession.hostCondition.value).toBe('host2');
        expect(memorySession.rankCondition.value).toBe(0);
    });

    it('should handle rank value change correctly', () => {
        const session = createMockSession;
        const memorySession = createMockMemorySession;
        memorySession.rankCondition = {
            value: 0,
            options: [
                { value: 0, rankInfo: { rankId: 'rank1' } },
                { value: 1, rankInfo: { rankId: 'rank2' } },
            ],
        };
        const strategy = createMockStrategy(true);

        const { getByTestId } = render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        const rankSelect = getByTestId('select-rankId');
        fireEvent.change(rankSelect, { target: { value: 1 } });

        expect(memorySession.rankCondition.value).toBe('1');
    });

    it('should handle group by change correctly', () => {
        const session = createMockSession;
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy(true);

        const { getByTestId } = render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        const groupSelect = getByTestId('select-groupId');
        fireEvent.change(groupSelect, { target: { value: GroupBy.COMPONENT } });

        expect(memorySession.groupId).toBe(GroupBy.COMPONENT);
    });

    it('should disable host and rank selects when in compare mode', () => {
        const session = createMockSession;
        session.compareRank.isCompare = true;
        session.compareRank.rankId = 'rank1';
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy(true);

        render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        expect(screen.getByTestId('select-host')).toBeDisabled();
        expect(screen.getByTestId('select-rankId')).toBeDisabled();
        expect(screen.getByTestId('select-groupId')).not.toBeDisabled();
    });

    it('should update host and rank conditions when session memoryCardInfos change', async () => {
        const session = createMockSession;
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy(true);

        const { rerender } = render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        const updatedSession = createMockSession;
        updatedSession.memoryCardInfos = ['new-card-info'];

        rerender(
            <MemoryHeader
                strategy={strategy}
                session={updatedSession}
                memorySession={memorySession}
            />,
        );

        await waitFor(() => {
            expect(memorySession.hostCondition.options).toEqual(['host1', 'host2']);
        });
    });

    it('should update rank options when isAllMemoryCompletedSwitch changes', async () => {
        const session = createMockSession;
        session.isAllMemoryCompletedSwitch = true;
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy(true);

        const { rerender } = render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        const updatedSession = createMockSession;
        updatedSession.isAllMemoryCompletedSwitch = true;

        rerender(
            <MemoryHeader
                strategy={strategy}
                session={updatedSession}
                memorySession={memorySession}
            />,
        );

        await waitFor(() => {
            expect(memorySession.rankCondition.options).toEqual([...memorySession.rankCondition.options]);
        });
    });

    it('should update conditions when compareRank.rankId changes', async () => {
        const session = createMockSession;
        session.compareRank.isCompare = false;
        session.compareRank.rankId = 'host1-rank1';
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy(true);

        const { rerender } = render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        const updatedSession = createMockSession;
        updatedSession.compareRank.isCompare = false;
        updatedSession.compareRank.rankId = 'host2-rank2';

        rerender(
            <MemoryHeader
                strategy={strategy}
                session={updatedSession}
                memorySession={memorySession}
            />,
        );

        await waitFor(() => {
            expect(memorySession.hostCondition.value).toBe('host2');
        });
    });

    it('should change groupId to DEFAULT when in compare mode and groupId is STREAM', async () => {
        const session = createMockSession;
        session.compareRank.isCompare = true;
        session.compareRank.rankId = 'rank1';
        const memorySession = createMockMemorySession;
        memorySession.groupId = GroupBy.STREAM;
        const strategy = createMockStrategy(true);

        render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        await waitFor(() => {
            expect(memorySession.groupId).toBe(GroupBy.DEFAULT);
        });
    });

    it('should generate correct group by options for compare mode', () => {
        const session = createMockSession;
        session.compareRank.isCompare = true;
        session.compareRank.rankId = 'rank1';
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy(true);

        render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        const groupSelect = screen.getByTestId('select-groupId');
        const options = Array.from(groupSelect.querySelectorAll('option')).map(option => option.value);

        expect(options).toEqual(['Overall', 'Component']);
        expect(options).not.toContain('Stream');
    });

    it('should generate correct group by options for non-compare mode', () => {
        const session = createMockSession;
        session.compareRank.isCompare = false;
        session.compareRank.rankId = 'rank1';
        const memorySession = createMockMemorySession;
        const strategy = createMockStrategy(true);

        render(
            <MemoryHeader
                strategy={strategy}
                session={session}
                memorySession={memorySession}
            />,
        );

        const groupSelect = screen.getByTestId('select-groupId');
        const options = Array.from(groupSelect.querySelectorAll('option')).map(option => option.value);

        expect(options).toEqual(['Overall', 'Stream', 'Component']);
    });
});
