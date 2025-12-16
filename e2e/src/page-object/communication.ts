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

import { type FrameLocator, type Page, type Locator } from '@playwright/test';
import { FrameworkPage } from './framework';
import { TableHelpers } from '../components';

export class CommunicationPage {
    readonly page: Page;
    readonly communicationFrame: FrameLocator;
    readonly fullPage: Locator;
    readonly filters: Locator; // 筛选条件区域
    readonly hcclChart: Locator;
    readonly stepSelector: Locator;
    readonly communicationGroupSelector: Locator;
    readonly operatorNameSelector: Locator;
    readonly durationAnalysisRadio: Locator;
    readonly communicationMatrixRadio: Locator;
    readonly communicationMatrixTypeSelector: Locator;
    readonly fullmask: Locator;
    readonly communicationMatrixMinRangeInput: Locator;
    readonly communicationMatrixMaxRangeInput: Locator;
    readonly matrixChart: Locator;

    constructor(page: Page) {
        this.page = page;
        this.communicationFrame = page.frameLocator('#Communication');
        this.fullPage = this.communicationFrame.locator('#root');
        this.filters = this.communicationFrame.getByTestId('filters');
        this.hcclChart = this.communicationFrame.locator('#hccl');
        this.stepSelector = this.communicationFrame.locator('#step-select'); // 迭代id
        this.communicationGroupSelector = this.communicationFrame.locator('#communicationGroup-select'); // 通信域
        this.operatorNameSelector = this.communicationFrame.locator('#operatorName-select'); // 算子名称
        this.communicationMatrixRadio = this.communicationFrame.locator('input[type="radio"][value="CommunicationMatrix"]'); // 通信矩阵
        this.durationAnalysisRadio = this.communicationFrame.locator('input[type="radio"][value="CommunicationDurationAnalysis"]'); // 通信耗时分析
        this.communicationMatrixTypeSelector = this.communicationFrame.locator('#communicationMatrixType-select'); // 通信矩阵类型
        this.fullmask = this.communicationFrame.locator('.fullmask');
        this.communicationMatrixMinRangeInput = this.communicationFrame.getByTestId('communicationMatrixMinRangeInput');
        this.communicationMatrixMaxRangeInput = this.communicationFrame.getByTestId('communicationMatrixMaxRangeInput');
        this.matrixChart = this.communicationFrame.locator('#matrixchart');
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.clickTab('communication');
    }

    // 切换到通信耗时分析
    async switchDurationAnalysis(communicationMatrixRadio: Locator, durationAnalysisRadio: Locator): Promise<void> {
        await communicationMatrixRadio.click();
        await durationAnalysisRadio.click();
    }

    /**
     * 定位到表格查看详情按钮并跳转到带宽分析页
     */
    async toOperatorpage(page: Page, communicationFrame: FrameLocator): Promise<void> {
        const tableLocator = communicationFrame.getByTestId('dataAnalysisTable').locator('.ant-table-container > .ant-table-content > table').first();
        const dataAnalysisTable = new TableHelpers(page, tableLocator, communicationFrame);
        const td = await dataAnalysisTable.getCell(1, 11);
        const linkBtn = td.locator('button');
        await linkBtn.click();
    }
}
