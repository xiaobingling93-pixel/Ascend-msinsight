/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test as baseTest, expect, WebSocket } from '@playwright/test';
import { SummaryPage, FrameworkPage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener, waitForWebSocketEvent } from '@/utils';
import { SelectHelpers } from '@/components';
import { FilePath } from '@/utils/constants';

interface TestFixtures {
    summaryPage: SummaryPage;
    ws: Promise<WebSocket>;
}
const test = baseTest.extend<TestFixtures>({
    summaryPage: async ({ page }, use) => {
        const summaryPage = new SummaryPage(page);
        await use(summaryPage);
    },
    ws: async ({ page }, use) => {
        const ws = setupWebSocketListener(page);
        await use(ws);
    },
});

let waitFwdBwdTimelineResponse: Promise<unknown>;

test.describe('Summary', () => {
    test.beforeEach(async ({ page, summaryPage, ws }) => {
        const { loadingDialog } = new FrameworkPage(page);
        const { fullmask } = summaryPage;
        waitFwdBwdTimelineResponse = waitForWebSocketEvent(page, (res) => res?.type === 'response' && res?.command === 'parallelism/pipeline/fwdBwdTimeline');

        await page.goto('/');
        await importData(page);
        await summaryPage.goto();
        if (await loadingDialog.count()) {
            await loadingDialog.waitFor({ state: 'attached' });
        }
        if (await fullmask.count()) {
            await fullmask.waitFor({ state: 'hidden' });
        }
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    // 基础数据展示
    test('test_baseInfoDisplay', async ({ summaryPage }) => {
        const { summaryFrame } = summaryPage;
        const baseInfoContainer = summaryFrame.locator('.mi-collapsible-panel').first();

        await expect(baseInfoContainer).toHaveScreenshot('base-info.png', { maxDiffPixels: 300 });
    });

    // 配置并行策略，生成排布数据
    test('test_generateRanks', async ({ page, summaryPage }) => {
        const { parallelismGraph } = summaryPage;
        await summaryPage.configureParallel({ algorithm: 'Megatron-LM (tp-cp-ep-dp-pp)', ppSize: 2, tpSize: 2, dpSize: 2, cpSize: 2, epSize: 2 });

        await page.mouse.move(0, 0);
        await expect(parallelismGraph).toHaveScreenshot('arrangement.png', { maxDiffPixels: 300 });
    });

    // 切换不同维度视图
    test('test_dimensionViews_when_changeDimensionTab', async ({ page, summaryPage }) => {
        const { summaryFrame, parallelismGraph, parallelismGraphLoading } = summaryPage;
        const dimensionTabs = ['DP', 'DP + PP', 'DP + PP + CP', 'DP + PP + CP + TP'];

        for (const tab of dimensionTabs) {
            await summaryFrame.getByRole('tab', { name: tab, exact: true }).click();
            if (tab !== 'DP') {
                await parallelismGraphLoading.waitFor({ state: 'hidden' });
            }

            await page.mouse.move(0, 0);
            await expect(parallelismGraph).toHaveScreenshot(`dimension-${tab}.png`, { maxDiffPixels: 300 });
        }
    });

    // 着色
    test('test_heatmap_given_preparingIndicator', async ({ page, summaryPage }) => {
        const { summaryFrame, parallelismGraph } = summaryPage;
        const dataTypeSelect = summaryFrame.locator('#dataType');
        const dataTypeSelector = new SelectHelpers(page, dataTypeSelect, summaryFrame);

        await summaryPage.changeDimensionTo('tp');
        await dataTypeSelector.open();
        await dataTypeSelector.selectOption('Preparing');

        await expect(parallelismGraph).toHaveScreenshot('heatmap-default.png', { maxDiffPixels: 300 });

        const dyeingMinimumInput = summaryFrame.getByTestId('input-dyeing-minimum');
        const dyeingMaximumInput = summaryFrame.getByTestId('input-dyeing-maximum');
        await dyeingMinimumInput.fill('700');
        await dyeingMaximumInput.fill('900');

        await expect(parallelismGraph).toHaveScreenshot('heatmap-edit.png', { maxDiffPixels: 300 });
    });

    // 点击卡，显示连线，点击连线，显示对应性能图表
    test('test_linkLinesDisplay_when_clickRanks', async ({ page, summaryPage }) => {
        const { parallelismGraph, parallelismGraphPlaceholder, pipelineChart, performanceChart } = summaryPage;

        await summaryPage.changeDimensionTo('tp');
        // 点击卡 0
        await parallelismGraphPlaceholder.click({
            position: {
                x: 59,
                y: 60,
            },
        });

        await page.mouse.move(0, 0);
        await expect(parallelismGraph).toHaveScreenshot('link-line.png', { maxDiffPixels: 300 });

        // 点击 PP 连线
        await parallelismGraphPlaceholder.click({
            position: {
                x: 53,
                y: 97,
            },
        });
        await waitFwdBwdTimelineResponse;

        await page.mouse.move(0, 0);
        await expect(pipelineChart).toHaveScreenshot('pp-line-clicked.png', { maxDiffPixels: 300 });

        // 点击 DP 连线
        await parallelismGraphPlaceholder.click({
            position: {
                x: 160,
                y: 104,
            },
        });

        await page.mouse.move(0, 0);
        await expect(performanceChart).toHaveScreenshot('dp-line-clicked.png', { maxDiffPixels: 300 });
    });

    // 点击框，图表联动展示对应数据
    test('test_performanceChartDisplay_when_clickLinkFrames', async ({ page, summaryPage }) => {
        const { parallelismGraphPlaceholder, performanceChart } = summaryPage;

        await summaryPage.changeDimensionTo('tp');
        // 点击第一个 DP 框
        await parallelismGraphPlaceholder.click({
            position: {
                x: 165,
                y: 182,
            },
        });

        await page.mouse.move(0, 0);
        await expect(performanceChart).toHaveScreenshot('dp-frame-clicked.png', { maxDiffPixels: 300 });
    });

    // 测试性能数据图表根据筛选条件变化
    test('test_performanceChartDisplay_given_specificConditions', async ({ page, summaryPage }) => {
        const { selectStep, selectOrderBy, selectRankGroup, selectTop, performanceChart, summaryFrame } = summaryPage;
        const stepSelector = new SelectHelpers(page, selectStep, summaryFrame);
        const rankGroupSelector = new SelectHelpers(page, selectRankGroup, summaryFrame);
        const orderBySelector = new SelectHelpers(page, selectOrderBy, summaryFrame);
        const topSelector = new SelectHelpers(page, selectTop, summaryFrame);

        await summaryPage.changeDimensionTo('tp');
        await stepSelector.open();
        await stepSelector.selectOption('2');
        await rankGroupSelector.open();
        await rankGroupSelector.setValue('(1,9)');
        await rankGroupSelector.selectOption('(1,9)');
        await orderBySelector.open();
        await orderBySelector.selectOption('Preparing');
        await topSelector.open();
        await topSelector.selectOption('1');

        await page.mouse.move(0, 0);
        await expect(performanceChart).toHaveScreenshot('performance-chart-filtered.png', { maxDiffPixels: 300 });
    });

    // 点击性能数据图表的某张卡，Computing Detail、Communication Detail 加载对应数据
    test('test_computingDetailTableAndCommunicationDetailTable_when_clickSpecificRankInPerformanceChart', async ({ page, summaryPage }) => {
        const { performanceChart, statisticsTableContainer } = summaryPage;
        const performanceChartCanvas = performanceChart.locator('canvas');

        await summaryPage.changeDimensionTo('tp');
        await page.waitForTimeout(1000);
        // 点击 rank 3
        await performanceChartCanvas.click({
            position: {
                x: 693,
                y: 219,
            },
        });

        await page.mouse.move(0, 0);
        await expect(statisticsTableContainer).toHaveScreenshot('statistics-table.png');
    });

    test('test_computingDetailTableAndCommunicationDetailTable_DetailsAndOrderBy', async ({ page, summaryPage }) => {
        const { computingDetail, communicationDetail, summaryFrame } = summaryPage;

        await summaryPage.changeDimensionTo('tp');
        await page.waitForTimeout(1000);

        // 展开Details
        await summaryFrame.getByRole('row', { name: 'AI_CORE 157876 Details down' }).getByRole('button').click();
        await summaryFrame.getByRole('row', { name: 'Communication 266466 69898' }).getByRole('button').click();
        await page.mouse.move(0, 0);
        await expect(computingDetail).toHaveScreenshot('computing-details.png');
        await expect(communicationDetail).toHaveScreenshot('communication-details.png');

        // 展开Details后切换页签
        await summaryFrame.getByText('4', { exact: true }).first().click();
        await summaryFrame.getByText('5', { exact: true }).nth(1).click();
        await page.mouse.move(0, 0);
        await expect(computingDetail).toHaveScreenshot('computing-details-page-change.png');
        await expect(communicationDetail).toHaveScreenshot('communication-details-page-change.png');

        // 展开切换页签后测试排序
        await summaryFrame.locator('div').filter({ hasText: /^Duration\(μs\)$/ }).first().click();
        await summaryFrame.locator('div').filter({ hasText: /^Start Time\(ms\)$/ }).nth(1).click();
        await page.mouse.move(0, 0);
        await expect(computingDetail).toHaveScreenshot('computing-details-orderby.png');
        await expect(communicationDetail).toHaveScreenshot('communication-details-orderby.png');
    });
});

// 专家建议查询慢卡/慢分组功能
test.describe('Summary(Expert Advice Slow Rank)', () => {
    test.beforeEach(async ({ page, summaryPage, ws }) => {
        const { loadingDialog } = new FrameworkPage(page);
        const { fullmask } = summaryPage;

        await page.goto('/');
        await importData(page, FilePath.EXPERET_ADVICE);
        await summaryPage.goto();
        if (await loadingDialog.count()) {
            await loadingDialog.waitFor({ state: 'attached' });
        }
        if (await fullmask.count()) {
            await fullmask.waitFor({ state: 'hidden' });
        }
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    // 切换不同维度生成专家建议
    test('test_expertAdvice_when_changeDimensionTab', async ({ page, summaryPage }) => {
        const { summaryFrame, parallelismExpertAdvice, parallelismGraphLoading } = summaryPage;
        const dimensionTabs = ['DP + PP', 'DP + PP + CP', 'DP + PP + CP + TP'];
        await summaryPage.configureParallel({ algorithm: 'Megatron-LM (tp-cp-ep-dp-pp)', ppSize: 8, tpSize: 8, dpSize: 8, cpSize: 8, epSize: 8 });

        for (const tab of dimensionTabs) {
            await summaryFrame.getByRole('tab', { name: tab, exact: true }).click();
            if (tab !== 'DP') {
                await parallelismGraphLoading.waitFor({ state: 'hidden' });
            }

            await page.mouse.move(0, 0);
            await expect(parallelismExpertAdvice).toHaveScreenshot(`expert-advice-${tab}.png`, { maxDiffPixels: 300 });
        }
    });
});

// 专家负载均衡功能
test.describe('Summary(MoE)', () => {
    test.beforeEach(async ({ page, summaryPage, ws }) => {
        const { loadingDialog } = new FrameworkPage(page);
        const { fullmask } = summaryPage;

        await page.goto('/');
        await importData(page, FilePath.MOE_PROFILING);
        await summaryPage.goto();
        if (await loadingDialog.count()) {
            await loadingDialog.waitFor({ state: 'attached' });
        }
        if (await fullmask.count()) {
            await fullmask.waitFor({ state: 'hidden' });
        }
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    // 展示专家负载均衡-profiling数据
    test('test_MoE_balancing_chart_display', async ({ page, summaryPage }) => {
        const { modelLayerNumInput, moeChartPanel } = summaryPage;

        await page.mouse.move(0, 0);
        await expect(modelLayerNumInput).toHaveValue('61', { timeout: 60_000 });
        await expect(moeChartPanel).toHaveScreenshot('MoE-balancing-profiling.png');
    });
});
