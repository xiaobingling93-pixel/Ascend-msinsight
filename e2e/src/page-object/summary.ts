/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type { FrameLocator, Page, Locator } from '@playwright/test';
import { FrameworkPage } from './framework';

interface ParallelValue {
    algorithm: 'Megatron-LM (tp-cp-ep-dp-pp)' | 'Megatron-LM (tp-cp-pp-ep-dp)';
    ppSize: number;
    tpSize: number;
    dpSize: number;
    cpSize: number;
    epSize: number;
}

type DimensionType = 'tp' | 'pp' | 'cp' | 'dp';

export class SummaryPage {
    readonly page: Page;
    summaryFrame: FrameLocator;
    readonly fullmask: Locator;
    readonly btnGenerate: Locator;
    readonly parallelismGraph: Locator;
    readonly parallelismGraphLoading: Locator;
    readonly parallelismGraphPlaceholder: Locator;
    readonly communicationOverviewContainer: Locator;
    readonly pipelineChart: Locator;
    readonly performanceChart: Locator;
    readonly selectStep: Locator;
    readonly selectRankGroup: Locator;
    readonly selectOrderBy: Locator;
    readonly selectTop: Locator;
    readonly statisticsTableContainer: Locator;

    constructor(page: Page) {
        this.page = page;
        this.summaryFrame = page.frameLocator('#Summary');
        this.fullmask = this.summaryFrame.locator('.fullmask');
        this.btnGenerate = this.summaryFrame.getByRole('button', { name: 'Generate' });
        this.parallelismGraph = this.summaryFrame.locator('.parallelism-graph');
        this.parallelismGraphLoading = this.summaryFrame.getByTestId('parallelism-graph-loading');
        this.parallelismGraphPlaceholder = this.summaryFrame.getByTestId('parallelism-graph-placeholder');
        this.communicationOverviewContainer = this.summaryFrame.locator('#communication-overview-panel');
        this.pipelineChart = this.summaryFrame.getByTestId('pipeline-chart');
        this.performanceChart = this.summaryFrame.getByTestId('performance-chart');
        this.selectStep = this.summaryFrame.locator('#select-step');
        this.selectOrderBy = this.summaryFrame.locator('#select-order-by');
        this.selectRankGroup = this.summaryFrame.locator('#select-rank-group');
        this.selectTop = this.summaryFrame.locator('#select-top');
        this.statisticsTableContainer = this.summaryFrame.getByTestId('statistics-table-container');
    }

    async configureParallel(value: ParallelValue): Promise<void> {
        const config = this.summaryFrame.getByTestId('form-generate-parallelism');
        const algorithmInput = config.locator('.ant-select-selector');
        const ppSizeInput = config.locator('#ppSize');
        const tpSizeInput = config.locator('#tpSize');
        const dpSizeInput = config.locator('#dpSize');
        const cpSizeInput = config.locator('#cpSize');
        const epSizeInput = config.locator('#epSize');

        await algorithmInput.click();
        const optionsList = this.summaryFrame.locator('#algorithm_list + div');
        await optionsList.locator(`.ant-select-item-option[title='${value.algorithm}']`).click();

        await ppSizeInput.fill(`${value.ppSize}`);
        await tpSizeInput.fill(`${value.tpSize}`);
        await dpSizeInput.fill(`${value.dpSize}`);
        await cpSizeInput.fill(`${value.cpSize}`);
        await epSizeInput.fill(`${value.epSize}`);

        await this.btnGenerate.click();
    }

    async changeDimensionTo(dimension: DimensionType): Promise<void> {
        await this.summaryFrame.getByRole('tab', { name: `${dimension.toLocaleUpperCase()}` }).click();
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.clickTab('summary');
    }
}
