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
import * as React from 'react';
import { render, screen, fireEvent, act } from '@testing-library/react';
import { useTranslation } from 'react-i18next';
import { useRootStore } from '../../context/context';
import { GroupBy } from '../../entity/memorySession';
import { Label, useHit, useChartCharacter, convertTime } from '../Common'; // 请替换为实际文件路径
import '@testing-library/jest-dom';
import { safeStr } from '@insight/lib/utils';

// Mock 所有外部依赖
jest.mock('react-i18next', () => ({
    useTranslation: jest.fn(),
}));

jest.mock('../../context/context', () => ({
    useRootStore: jest.fn(),
}));

jest.mock('@insight/lib/icon', () => ({
    HelpIcon: ({ style, height, width, onClick }: any) => (
        <div
            data-testid="help-icon"
            style={style}
            data-height={height}
            data-width={width}
            onClick={onClick}
        >
            Help Icon
        </div>
    ),
}));

jest.mock('@insight/lib/components', () => ({
    Tooltip: ({ title, children }: any) => (
        <div data-testid="tooltip">
            <div data-testid="tooltip-title">{title}</div>
            {children}
        </div>
    ),
}));

// 测试 Label 组件
describe('Label Component', () => {
    it('should render label with name and colon', () => {
        render(<Label name="Test Label" />);

        expect(screen.getByText('Test Label :')).toBeInTheDocument();
    });

    it('should apply custom styles when provided', () => {
        const customStyle = { color: 'red', fontSize: '16px' };
        render(<Label name="Test Label" style={customStyle} />);

        const labelElement = screen.getByText('Test Label :');
        expect(labelElement).toHaveStyle('margin-right: 8px');
        expect(labelElement).toHaveStyle('color: red');
        expect(labelElement).toHaveStyle('font-size: 16px');
    });

    it('should render with React node as name', () => {
        const CustomNode = () => <span data-testid="custom-node">Custom Node</span>;
        render(<Label name={<CustomNode />} />);

        expect(screen.getByTestId('custom-node')).toBeInTheDocument();
        expect(screen.getByText('Custom Node')).toBeInTheDocument();
    });

    it('should handle undefined style prop', () => {
        render(<Label name="Test Label" style={undefined} />);

        const labelElement = screen.getByText('Test Label :');
        expect(labelElement).toHaveStyle('margin-right: 8px');
    });
});

// 测试 useHit Hook
describe('useHit Hook', () => {
    const mockT = jest.fn();

    beforeEach(() => {
        (mockT as jest.Mock).mockReturnValue((key) => {
            const translations: Record<string, string> = {
                'searchCriteria.Overall': 'Overall',
                'searchCriteria.OverallDescribe': 'Overall Description Text',
                'searchCriteria.Stream': 'Stream',
                'searchCriteria.StreamDescribe': 'Stream Description Text',
                'searchCriteria.Component': 'Component',
                'searchCriteria.ComponentDescribe': 'Component Description Text',
            };
            return translations[key] || key;
        });
        (useTranslation as jest.Mock).mockReturnValue({
            t: mockT,
            i18n: { language: 'en-US' },
        });
    });

    afterEach(() => {
        jest.clearAllMocks();
    });

    const TestComponent = () => {
        const hitComponent = useHit();
        return <div>{hitComponent}</div>;
    };

    it('should render help icon with correct props', () => {
        render(<TestComponent />);

        const helpIcon = screen.getByTestId('help-icon');
        expect(helpIcon).toBeInTheDocument();
        expect(helpIcon).toHaveStyle('cursor: pointer');
        expect(helpIcon).toHaveAttribute('data-height', '20');
        expect(helpIcon).toHaveAttribute('data-width', '20');
    });

    it('should call translation function with correct keys', () => {
        render(<TestComponent />);

        expect(mockT).toHaveBeenCalledWith('searchCriteria.Overall');
        expect(mockT).toHaveBeenCalledWith('searchCriteria.OverallDescribe');
        expect(mockT).toHaveBeenCalledWith('searchCriteria.Stream');
        expect(mockT).toHaveBeenCalledWith('searchCriteria.StreamDescribe');
        expect(mockT).toHaveBeenCalledWith('searchCriteria.Component');
        expect(mockT).toHaveBeenCalledWith('searchCriteria.ComponentDescribe');
    });

    it('should render tooltip with correct content structure', () => {
        render(<TestComponent />);

        const tooltipTitle = screen.getByTestId('tooltip-title');
        expect(tooltipTitle).toBeInTheDocument();
    });
});

// 测试 useChartCharacter Hook
describe('useChartCharacter Hook', () => {
    const mockT = jest.fn();
    const mockMemoryStore = {
        activeSession: undefined,
    };

    beforeEach(() => {
        mockT.mockImplementation((key: string, options?: { returnObjects: boolean }) => {
            if (options?.returnObjects) {
                if (key === 'searchCriteria.CurveDescribe') {
                    return ['Curve description line 1', 'Curve description line 2'];
                }
                if (key === 'searchCriteria.CurveDescribeByCompenent') {
                    return ['Component curve line 1', 'Component curve line 2'];
                }
                return [];
            }
            return key;
        });

        (useTranslation as jest.Mock).mockReturnValue({
            t: mockT,
        });

        (useRootStore as jest.Mock).mockReturnValue({
            memoryStore: mockMemoryStore,
        });
    });

    afterEach(() => {
        jest.clearAllMocks();
    });

    const TestComponent = () => {
        const chartCharacter = useChartCharacter();
        return <div>{chartCharacter}</div>;
    };

    it('should render help icon with correct props', () => {
        render(<TestComponent />);

        const helpIcon = screen.getByTestId('help-icon');
        expect(helpIcon).toBeInTheDocument();
        expect(helpIcon).toHaveStyle('cursor: pointer');
        expect(helpIcon).toHaveAttribute('data-height', '20');
        expect(helpIcon).toHaveAttribute('data-width', '20');
    });

    it('should handle undefined memory session', async () => {
        mockMemoryStore.activeSession = undefined;

        await act(async () => {
            render(<TestComponent />);
        });

        expect(mockT).not.toHaveBeenCalledWith('searchCriteria.CurveDescribe', expect.any(Object));
        expect(mockT).not.toHaveBeenCalledWith('searchCriteria.CurveDescribeByCompenent', expect.any(Object));
    });

    it('should load CurveDescribe content for DEFAULT group', async () => {
        mockMemoryStore.activeSession = {
            groupId: GroupBy.DEFAULT,
        };

        await act(async () => {
            render(<TestComponent />);
        });

        expect(mockT).toHaveBeenCalledWith('searchCriteria.CurveDescribe', { returnObjects: true });
    });

    it('should load CurveDescribeByCompenent content for COMPONENT group', async () => {
        mockMemoryStore.activeSession = {
            groupId: GroupBy.COMPONENT,
        };

        await act(async () => {
            render(<TestComponent />);
        });

        expect(mockT).toHaveBeenCalledWith('searchCriteria.CurveDescribeByCompenent', { returnObjects: true });
    });

    it('should handle unknown group ID by returning empty array', async () => {
        mockMemoryStore.activeSession = {
            groupId: 'UNKNOWN_GROUP',
        };

        await act(async () => {
            render(<TestComponent />);
        });

        // 对于未知的groupId，应该返回空数组，不调用t函数
        expect(mockT).not.toHaveBeenCalledWith('searchCriteria.CurveDescribe', expect.any(Object));
        expect(mockT).not.toHaveBeenCalledWith('searchCriteria.CurveDescribeByCompenent', expect.any(Object));
    });

    it('should update content when groupId changes', async () => {
        // 初始渲染为DEFAULT组
        mockMemoryStore.activeSession = {
            groupId: GroupBy.DEFAULT,
        };

        const { rerender } = await act(async () => {
            return render(<TestComponent />);
        });

        expect(mockT).toHaveBeenCalledWith('searchCriteria.CurveDescribe', { returnObjects: true });

        // 更改为COMPONENT组
        mockMemoryStore.activeSession = {
            groupId: GroupBy.COMPONENT,
        };

        await act(async () => {
            rerender(<TestComponent />);
        });

        expect(mockT).toHaveBeenCalledWith('searchCriteria.CurveDescribeByCompenent', { returnObjects: true });
    });

    it('should handle empty hit array gracefully', async () => {
        mockMemoryStore.activeSession = {
            groupId: 'UNKNOWN_GROUP', // 这会返回空数组
        };

        await act(async () => {
            render(<TestComponent />);
        });

        // 组件应该正常渲染，不会因为空数组而崩溃
        expect(screen.getByTestId('help-icon')).toBeInTheDocument();
    });
});

// 测试边缘情况
describe('Edge Cases', () => {
    it('should handle empty string translations', () => {
        const mockT = jest.fn(() => '');
        (useTranslation as jest.Mock).mockReturnValue({
            t: mockT,
        });

        const TestComponent = () => {
            const hitComponent = useHit();
            return <div>{hitComponent}</div>;
        };

        // 应该不会因为空字符串而崩溃
        expect(() => render(<TestComponent />)).not.toThrow();
    });

    it('should handle null or undefined translations gracefully', () => {
        const mockT = jest.fn(() => null);
        (useTranslation as jest.Mock).mockReturnValue({
            t: mockT,
        });

        const TestComponent = () => {
            const hitComponent = useHit();
            return <div>{hitComponent}</div>;
        };

        // 应该不会因为null而崩溃
        expect(() => render(<TestComponent />)).not.toThrow();
    });
});

/**
 * 生成指定长度的随机字符串，使用字符串列表和随机数对应的方式进行匹配
 * @param length 字符串长度
 * @returns 随机字符串
 */
const generateRandomString = (length: number): string => {
    let result = '';
    const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    for (let i = 0; i < length; i++) {
        result += characters.charAt(Math.floor(Math.random() * characters.length));
    }
    return result;
};

// 测试Label组件是否能正确显示随机文本
it('test function Label with random string then show correct text', () => {
    const randomName = generateRandomString(10);
    render(<Label name={randomName} />);
    expect(screen.getByText(`${randomName} :`)).toBeDefined();
});

// 测试数字类型时间转换为HH:MM:SS.MS.US格式
it('test function convertTime with different inputs return correct results', () => {
    const nullInput = null;
    expect(convertTime(nullInput)).toBe('');
    const notNumStringInput = `a${generateRandomString(5)}`;
    expect(convertTime(notNumStringInput)).toBe(notNumStringInput);
    const hour = Math.floor(Math.random() * 10000);
    const minute = Math.floor(Math.random() * 60);
    const second = Math.floor(Math.random() * 60);
    const milliSecond = Math.floor(Math.random() * 1000);
    const microSecond = Math.floor(Math.random() * 1000);
    const numStringInput = `${(hour * 1000 * 60 * 60) + (minute * 1000 * 60) + (second * 1000) + milliSecond}.${String(microSecond).padStart(3, '0')}`;
    expect(convertTime(numStringInput)).toBe(`${String(hour).padStart(2, '0')}:${String(minute).padStart(2, '0')}:${String(second).padStart(2,
        '0')}.${String(milliSecond).padStart(3, '0')}.${String(microSecond).padStart(3, '0')}`);
});
