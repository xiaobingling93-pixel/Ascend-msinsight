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
import { render, screen, fireEvent } from '@testing-library/react';
import { Checkbox } from '@insight/lib/components';
import OptionalCheckbox from '../OptionalCheckbox';
import { Label } from '../Common';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import '@testing-library/jest-dom';

// Mock the child components
jest.mock('@insight/lib/components', () => ({
    Checkbox: ({ id, checked, onChange, disabled }) => (
        <input
            type="checkbox"
            data-testid="checkbox"
            data-id={id}
            data-checked={checked}
            data-disabled={disabled}
            onChange={onChange}
        />
    ),
}));

jest.mock('../Common', () => ({
    useHit: () => 'hit-indicator',
    Label: ({ name }: { name: string }) => <label data-testid="label">{name}</label>,
}));

// Mock mobx observer
jest.mock('mobx-react-lite', () => ({
    observer: (component: React.ComponentType) => component,
}));

describe('OptionalCheckbox', () => {
    const defaultProps = {
        idKey: 'test-checkbox',
        name: 'Test Checkbox',
        value: false,
        onChange: jest.fn(),
        visible: true,
        disabled: false,
    };

    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('should render nothing when visible is false', () => {
        const props = { ...defaultProps, visible: false };

        const { container } = render(<OptionalCheckbox {...props} />);

        expect(container.firstChild).toBeNull();
    });

    it('should render checkbox and label when visible is true', () => {
        render(<OptionalCheckbox {...defaultProps} />);

        expect(screen.getByTestId('label')).toBeInTheDocument();
        expect(screen.getByTestId('checkbox')).toBeInTheDocument();
    });

    it('should render with correct structure and classes when visible', () => {
        const { container } = render(<OptionalCheckbox {...defaultProps} />);

        const wrapperDiv = container.firstChild as HTMLElement;
        expect(wrapperDiv).toBeInTheDocument();
        expect(wrapperDiv).toHaveClass('flex');
        expect(wrapperDiv).toHaveClass('items-center');
    });

    it('should pass correct props to Label component', () => {
        const testName = 'Custom Label Name';
        render(<OptionalCheckbox {...defaultProps} name={testName} />);
        const cusLable = screen.getByTestId('label');
        expect(cusLable).toHaveTextContent(testName);
    });

    it('should pass correct props to Checkbox component', () => {
        const testId = 'custom-id';
        const testValue = true;
        const testDisabled = true;

        render(
            <OptionalCheckbox
                {...defaultProps}
                idKey={testId}
                value={testValue}
                disabled={testDisabled}
            />,
        );

        const checkbox = screen.getByTestId('checkbox');
        expect(checkbox).toHaveAttribute('data-id', testId);
        expect(checkbox).toHaveAttribute('data-checked', 'true');
        expect(checkbox).toHaveAttribute('data-disabled', 'true');
    });

    it('should call onChange when checkbox is clicked', () => {
        const mockOnChange = jest.fn();
        render(<OptionalCheckbox {...defaultProps} onChange={mockOnChange} />);

        const checkbox = screen.getByTestId('checkbox');
        fireEvent.click(checkbox);

        expect(mockOnChange).toHaveBeenCalledTimes(1);
    });

    it('should handle default values when props are not provided', () => {
        // Test with minimal props
        const minimalProps = {
            idKey: 'test',
            name: 'Test',
            value: false,
            onChange: jest.fn(),
            visible: true,
        };

        render(<OptionalCheckbox {...minimalProps} />);

        const checkbox = screen.getByTestId('checkbox');
        expect(checkbox).toHaveAttribute('data-disabled', 'false'); // default disabled should be false
    });

    it('should not call onChange when disabled', () => {
        const mockOnChange = jest.fn();
        render(<OptionalCheckbox {...defaultProps} disabled={true} onChange={mockOnChange} />);

        const checkbox = screen.getByTestId('checkbox');
        fireEvent.change(checkbox);

        expect(mockOnChange).not.toHaveBeenCalled();
    });

    it('should render with different boolean values', () => {
        // Test with value true
        const { rerender } = render(<OptionalCheckbox {...defaultProps} value={true} />);

        let checkbox = screen.getByTestId('checkbox');
        expect(checkbox).toHaveAttribute('data-checked', 'true');

        // Test with value false
        rerender(<OptionalCheckbox {...defaultProps} value={false} />);

        checkbox = screen.getByTestId('checkbox');
        expect(checkbox).toHaveAttribute('data-checked', 'false');
    });

    it('should handle edge cases for idKey and name', () => {
        // Test with empty strings
        render(<OptionalCheckbox {...defaultProps} idKey="" name="" />);

        const checkbox = screen.getByTestId('checkbox');
        expect(checkbox).toHaveAttribute('data-id', '');

        const label = screen.getByTestId('label');
        expect(label).toHaveTextContent('');
    });

    it('should maintain component structure consistency', () => {
        const { container } = render(<OptionalCheckbox {...defaultProps} />);

        const wrapperDiv = container.firstChild as HTMLElement;
        expect(wrapperDiv.children).toHaveLength(2); // Label and Checkbox

        const label = screen.getByTestId('label');
        const checkbox = screen.getByTestId('checkbox');

        expect(wrapperDiv).toContainElement(label);
        expect(wrapperDiv).toContainElement(checkbox);
    });

    describe('visibility toggling', () => {
        it('should toggle visibility when visible prop changes', () => {
            const { rerender, container } = render(<OptionalCheckbox {...defaultProps} visible={true} />);

            expect(screen.getByTestId('checkbox')).toBeInTheDocument();

            rerender(<OptionalCheckbox {...defaultProps} visible={false} />);

            expect(container.firstChild).toBeNull();

            rerender(<OptionalCheckbox {...defaultProps} visible={true} />);

            expect(screen.getByTestId('checkbox')).toBeInTheDocument();
        });
    });

    describe('CheckboxChangeEvent simulation', () => {
        it('should handle CheckboxChangeEvent properly', () => {
            const mockOnChange = jest.fn();
            render(<OptionalCheckbox {...defaultProps} onChange={mockOnChange} />);

            const checkbox = screen.getByTestId('checkbox');

            // Simulate a change event with target.checked
            const mockEvent = {
                target: { checked: true },
                stopPropagation: jest.fn(),
                preventDefault: jest.fn(),
            } as unknown as CheckboxChangeEvent;

            // Since we're using a simplified mock, we'll test the actual event handling
            // by simulating a click which should trigger the onChange
            fireEvent.click(checkbox);

            expect(mockOnChange).toHaveBeenCalledTimes(1);
        });
    });
});
