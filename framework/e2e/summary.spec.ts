/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { test as baseTest, expect } from '@playwright/test';
import { SummaryPage, FrameworkPage } from './page-object';
import { clearAllData, importData } from './utils';

interface TestFixtures {
    summaryPage: SummaryPage;
}
const test = baseTest.extend<TestFixtures>({
    summaryPage: async ({ page }, use) => {
        const summaryPage = new SummaryPage(page);
        await use(summaryPage);
    },
});

test.describe('Summary', () => {
    test.beforeEach(async ({ page, summaryPage }) => {
        const { loadingDialog } = new FrameworkPage(page);
        const { fullmask, chartContainer } = summaryPage;
        await page.goto('/');
        await importData(page);
        await summaryPage.goto();
        if (await loadingDialog.count()) {
            await loadingDialog.waitFor({ state: 'attached' });
        }
        await chartContainer.innerHTML();
        if (await fullmask.count()) {
            await fullmask.waitFor({ state: 'hidden' });
        }
    });

    // 基本数据是否加载
    test('load success', async ({ page, summaryPage }) => {
        const { summaryFrame, chartContainer } = summaryPage;
        const filePathDom = summaryFrame.locator('.content').first();
        await filePathDom.waitFor({ state: 'visible' });
        const filePath = await filePathDom.innerText();
        expect(filePath.length > 0).toBe(true);
        await page.mouse.move(0, 0);
        try {
            await expect(chartContainer).toHaveScreenshot('parallelChart1.png');
        } catch {
            await expect(chartContainer).toHaveScreenshot('parallelChart1-1.png');
        }
    });

    // 配置并行策略
    test('configure parallel strategy', async ({ page, summaryPage }) => {
        const {
            summaryFrame,
            btnGenerate,
            parallelRankContainer,
            configureParallel,
            configureParallelSwitch,
        } = summaryPage;

        if (await btnGenerate.isDisabled()) {
            return;
        }

        await configureParallel(
            summaryFrame,
            btnGenerate,
            { algorithm: 'Megatron-LM(tp-dp-pp)', ppSize: 2, tpSize: 2, dpSize: 2 },
        );
        await page.mouse.move(0, 0);
        await expect(parallelRankContainer).toHaveScreenshot('parallelRank.png');

        await configureParallelSwitch(
            page,
            summaryFrame,
            {
                pipelineParallel: true,
                tensorParallel: true,
                dataParallel: true,
                dataType: 'Preparing',
                dyeingStep: 0.05,
            });
        await page.mouse.move(0, 0);
        await expect(parallelRankContainer).toHaveScreenshot('parallelRank2.png');
    });

    // 点击rank，显示连线
    test('click rank', async ({ page, summaryPage }) => {
        const {
            summaryFrame,
            btnGenerate,
            parallelRankContainer,
            configureParallel,
        } = summaryPage;

        if (await btnGenerate.isDisabled()) {
            return;
        }
        await configureParallel(
            summaryFrame,
            btnGenerate,
            { algorithm: 'Megatron-LM(tp-dp-pp)', ppSize: 2, tpSize: 2, dpSize: 2 },
        );

        const rank = parallelRankContainer.locator('.rank[rank-id="4"]');
        await rank.click();
        await page.mouse.move(0, 0);
        await expect(parallelRankContainer).toHaveScreenshot('parallelRank3.png');
    });

    // 点击rank连线，图表联动展示对应数据
    test('click link', async ({ page, summaryPage }) => {
        const {
            summaryFrame,
            btnGenerate,
            parallelRankContainer,
            chartContainer,
            parallelDrawLineSVG,
            stageChartContainer,
            rankChartContainer,
            configureParallel,
            clickLine,
        } = summaryPage;

        if (await btnGenerate.isDisabled()) {
            return;
        }
        await configureParallel(
            summaryFrame,
            btnGenerate,
            { algorithm: 'Megatron-LM(tp-dp-pp)', ppSize: 2, tpSize: 2, dpSize: 2 },
        );
        const rank = parallelRankContainer.locator('.rank[rank-id="4"]');
        await rank.click();

        await clickLine(parallelDrawLineSVG, 'horizontalLine');
        await page.mouse.move(0, 0);
        await expect(chartContainer).toHaveScreenshot('parallelChartClickHorizontalLine.png');

        await clickLine(parallelDrawLineSVG, 'dpLine');
        await page.mouse.move(0, 0);
        await expect(chartContainer).toHaveScreenshot('parallelChartClickDpLine.png');

        await clickLine(parallelDrawLineSVG, 'verticalLine');
        await page.mouse.move(0, 0);
        await expect(stageChartContainer).toHaveScreenshot('stageChartClickVerticalLine.png');
        await expect(rankChartContainer).toHaveScreenshot('rankChartClickVerticalLine.png');
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });
});
