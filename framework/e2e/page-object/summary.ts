/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type { FrameLocator, Page, Locator } from '@playwright/test';
import { FrameworkPage } from './framework';
import { CheckboxHelpers, SelectHelpers } from '../components';


interface ParallelValue {
    algorithm: string;
    ppSize: number;
    tpSize: number;
    dpSize: number;
};

interface ParallelSwitchValue {
    pipelineParallel: boolean;
    tensorParallel: boolean;
    dataParallel: boolean;
    dataType: string;
    dyeingStep?: number;
};

export class SummaryPage {
    readonly page: Page;
    readonly summaryFrame: FrameLocator;
    readonly fullmask: Locator;
    readonly btnGenerate: Locator;
    readonly chartContainer: Locator;
    readonly parallelRankContainer: Locator;
    readonly parallelDrawLineSVG: Locator;
    readonly stageChartContainer: Locator;
    readonly rankChartContainer: Locator;

    constructor(page: Page) {
        this.page = page;
        this.summaryFrame = page.frameLocator('#Summary');
        this.fullmask = this.summaryFrame.locator('.fullmask');
        this.btnGenerate = this.summaryFrame.locator('button').first();
        this.chartContainer = this.summaryFrame.locator('#overview-chart > div').first();
        this.parallelRankContainer = this.summaryFrame.getByTestId('parallelRankContainer');
        this.parallelDrawLineSVG = this.summaryFrame.locator('#parallelDrawLineSVG');
        this.stageChartContainer = this.summaryFrame.locator('#STAGE');
        this.rankChartContainer = this.summaryFrame.locator('#RANK');
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

    async configureParallelSwitch(page: Page, summaryFrame: FrameLocator, value: ParallelSwitchValue): Promise<void> {
        const config = summaryFrame.getByTestId('parallelSwitch');

        if (value.pipelineParallel) {
            const pipelineParallelCheckbox = new CheckboxHelpers(
                page,
                config.locator('#pipelineParallel'),
                summaryFrame,
            );
            await pipelineParallelCheckbox.click();
        }

        if (value.tensorParallel) {
            const tensorParallelCheckbox = new CheckboxHelpers(
                page,
                config.locator('#tensorParallel'),
                summaryFrame,
            );
            await tensorParallelCheckbox.click();
        }

        if (value.dataParallel) {
            const dataParallelCheckbox = new CheckboxHelpers(
                page,
                config.locator('#dataParallel'),
                summaryFrame,
            );
            await dataParallelCheckbox.click();
        }

        const dataTypeSelect = new SelectHelpers(page, config.locator('#dataType'), summaryFrame);
        await dataTypeSelect.open();
        await dataTypeSelect.selectOption(value.dataType);

        if (value.dataType !== 'None') {
            const dyeingStepInput = config.locator('#dyeingStep');
            await dyeingStepInput.fill(`${value.dyeingStep}`);
        }
    }

    async clickLine(lineSVG: Locator, gClass: string): Promise<void> {
        const lineContainer = lineSVG.locator(`.${gClass}`);
        const line = lineContainer.locator('polyline').first();
        await line.click({ force: true });
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.clickTab('summary');
    }
}
