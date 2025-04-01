/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
