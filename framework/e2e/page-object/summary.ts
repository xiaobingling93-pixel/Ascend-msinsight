/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type { FrameLocator, Page, Locator } from '@playwright/test';
import { FrameworkPage } from './framework';


interface ParallelValue {
    algorithm: string;
    ppSize: number;
    tpSize: number;
    dpSize: number;
};

export class SummaryPage {
    readonly page: Page;
    readonly summaryFrame: FrameLocator;
    readonly fullmask: Locator;
    readonly btnGenerate: Locator;

    constructor(page: Page) {
        this.page = page;
        this.summaryFrame = page.frameLocator('#Summary');
        this.fullmask = this.summaryFrame.locator('fullmask');
        this.btnGenerate = this.summaryFrame.locator('button').first();
    }

    async configureParallel(summaryFrame: FrameLocator, btnGenerate: Locator, value: ParallelValue): Promise<void> {
        const config = summaryFrame.locator('.CommunicatorHeader');
        const algorithmInput = config.locator('.ant-select-selector');
        const ppSizeInput = config.locator('#ppSize');
        const tpSizeInput = config.locator('#tpSize');
        const dpSizeInput = config.locator('#dpSize');

        await algorithmInput.click();
        const optionsList = summaryFrame.locator('#algorithm_list + div');
        await optionsList.locator(`.ant-select-item-option[title='${value.algorithm}']`).click();

        await ppSizeInput.fill(`${value.ppSize}`);
        await tpSizeInput.fill(`${value.tpSize}`);
        await dpSizeInput.fill(`${value.dpSize}`);

        await btnGenerate.click();
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.clickTab('summary');
    }
}
