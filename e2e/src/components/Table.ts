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
import { Locator } from '@playwright/test';
import { Component } from './Component';

export class TableHelpers extends Component {
    /**
     * 获取表格行数
     */
    async getTableRows(): Promise<number> {
        return this.locator.locator('tbody tr').count();
    }

    /**
     * 定位到某个单元格
     */
    async getCell(row: number, column: number): Promise<Locator> {
        return this.locator.locator(`tbody tr:nth-child(${row}) td:nth-child(${column})`);
    }

    /**
     * 表头排序
     */
    async sortTableHead(tableHeadText: string): Promise<void> {
        await this.locator.getByText(tableHeadText).click();
    }

    /**
     * 获取表格列数
     */
    async getTableColumns(): Promise<number> {
        return this.locator.locator('thead tr th').count();
    }
}
