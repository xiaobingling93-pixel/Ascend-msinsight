/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { render, screen, fireEvent } from '@testing-library/react';
import { InputOption, handleTimestampOffsetReassignment } from './offsetConfig';
import type { InsightUnit } from '../../../entity/insight';

// Mock translations
jest.mock('react-i18next', () => ({
    useTranslation: (): { t: (key: string) => string } => ({
        t: (key: string): string => key,
    }),
}));

jest.mock('@emotion/react', () => ({
    useTheme: (): Record<string, any> => ({
        buttonColor: {},
    }),
}));

const mockMetaData = {
    cardId: 'cardId1',
};

const mockMetaData2 = {
    cardId: 'cardId2',
};

beforeEach((): void => {
    session.unitsConfig.offsetConfig.timestampOffset = {
        cardId1: 100,
    };
    session.selectedUnits = [
        { alignStartTimestamp: 123456789 } as InsightUnit,
    ];
    session.units = [{ metadata: mockMetaData } as InsightUnit];
});

describe('Function handleTimestampOffsetReassignment Check', () => {
    it('normal', () => {
        session.unitsConfig.offsetConfig.timestampOffset = {
            cardId__1: 100,
        };
        expect(handleTimestampOffsetReassignment(session, {
            cardId: 'cardId',
            processId: '111',
            threadId: '',
            startTime: 0,
            endTime: 1,
        }, 0)).toBe(false);
    });

    it('with in cardMetaData.processId = undefined', () => {
        session.unitsConfig.offsetConfig.timestampOffset = {
            cardId__1: 100,
        };
        expect(handleTimestampOffsetReassignment(session, {
            cardId: 'cardId',
            processId: (undefined as unknown) as string,
            threadId: '',
            startTime: 0,
            endTime: 1,
        }, 0)).toBe(true);
    });

    it('with in cardMetaData.processId = null', () => {
        session.unitsConfig.offsetConfig.timestampOffset = {
            cardId__1: 100,
        };
        expect(handleTimestampOffsetReassignment(session, {
            cardId: 'cardId',
            processId: (null as unknown) as string,
            threadId: '',
            startTime: 0,
            endTime: 1,
        }, 0)).toBe(true);
    });

    it('with in cardMetaData.processId = \'\'', () => {
        session.unitsConfig.offsetConfig.timestampOffset = {
            cardId__1: 100,
        };
        expect(handleTimestampOffsetReassignment(session, {
            cardId: 'cardId',
            processId: '',
            threadId: '',
            startTime: 0,
            endTime: 1,
        }, 0)).toBe(true);
    });
});

describe('Timestamp Offset Component', () => {
    // 无offset输入框初始值验证
    it('renders InputOption component without initial value', () => {
        render(<InputOption session={session} metaData={mockMetaData2} />);
        const btnOffset = screen.getByText('Offset');
        fireEvent.click(btnOffset);
        const inputElement = screen.getByDisplayValue('0');
        expect(inputElement).toBeInTheDocument();
    });
    // 输入框初始值验证
    it('renders InputOption component with initial value', () => {
        render(<InputOption session={session} metaData={mockMetaData} />);
        const btnOffset = screen.getByText('Offset');
        fireEvent.click(btnOffset);
        const inputElement = screen.getByDisplayValue('100');
        expect(inputElement).toBeInTheDocument();
    });

    // 输入框值更改验证
    it('updates input value on change', () => {
        render(<InputOption session={session} metaData={mockMetaData} />);
        const btnOffset = screen.getByText('Offset');
        fireEvent.click(btnOffset);
        const inputElement = screen.getByDisplayValue('100');
        fireEvent.change(inputElement, { target: { value: '200' } });
        expect(inputElement).toHaveValue('200');
    });

    // 输入非数字时，报错提示验证
    it('displays tooltip with title on invalid input', () => {
        render(<InputOption session={session} metaData={mockMetaData} />);
        const btnOffset = screen.getByText('Offset');
        fireEvent.click(btnOffset);
        const inputElement = screen.getByDisplayValue('100');
        fireEvent.change(inputElement, { target: { value: 'invalid' } });
        const tooltip = screen.getByText('headerButtonTooltip:TimeStampOffset');
        expect(tooltip).toBeInTheDocument();
    });

    // 出入非数字值，失焦时，重置为默认值验证
    it('resets value to default when blurred with invalid input', () => {
        render(<InputOption session={session} metaData={mockMetaData} />);
        const btnOffset = screen.getByText('Offset');
        fireEvent.click(btnOffset);
        const inputElement = screen.getByDisplayValue('100');
        fireEvent.change(inputElement, { target: { value: 'invalid' } });
        fireEvent.blur(inputElement);
        expect(inputElement).toHaveValue('0');
    });

    // 点击AlignToStart图标获取对齐偏移量验证
    it('handles align start button click', () => {
        render(<InputOption session={session} metaData={mockMetaData} />);
        const btnOffset = screen.getByText('Offset');
        fireEvent.click(btnOffset);
        const buttonElement = screen.getByLabelText('align to start');
        fireEvent.click(buttonElement);
        const inputElement = screen.getByDisplayValue('123456789');
        expect(inputElement).toBeInTheDocument();
    });
});
