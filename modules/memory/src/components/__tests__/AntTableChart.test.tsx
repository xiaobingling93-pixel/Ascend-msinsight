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
import { render, screen } from '@testing-library/react';
import { tableData, tableConfig } from '../__mock__/table.mock';
import { AntTableChart } from '../AntTableChart';

// AntTableChart组件测试，通过构造模拟数据和指定配置项进行测试
describe('test Table in memory', () => {
    const testTable = <AntTableChart tableData={tableData} isCompare={false} {...tableConfig} />;
    beforeAll(() => {
        // To fix the error 'TypeError: window.matchMedia is not a function'
        Object.defineProperty(window, 'matchMedia', {
            value: () => {
                return {
                    matches: false,
                    onchange: null,
                    addListener: jest.fn(),
                    removeListener: jest.fn(),
                    addEventListener: jest.fn(),
                    removeEventListener: jest.fn(),
                    dispatchEvent: jest.fn(),
                };
            },
        });
    });

    // 测试表头是否正确渲染
    it('renders table headers correctly', () => {
        render(testTable);
        const columns = screen.getAllByRole('columnheader');
        expect(columns.length - 1).toBe(tableData.columns.length);
        tableData.columns.forEach(column => expect(screen.getByText(column.name)).toBeDefined());
    });

    // 测试表格数据行数是否正确，每行数据是否已显示
    it('renders table rows correctly', () => {
        render(testTable);
        const rows = screen.getAllByRole('row');
        expect(rows.length).toBe(tableData.rows.length + 1);
        tableData.rows.forEach((row) => {
            Object.values(row).forEach(value => expect(screen.getAllByText(value)).toBeDefined());
        });
    });
});
