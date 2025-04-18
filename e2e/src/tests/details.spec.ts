/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import { expect, test as baseTest } from '@playwright/test';
import { DetailsPage, FrameworkPage } from '@/page-object';
import { clearAllData, importData, setCompare, waitForAllParsed } from '@/utils';
import { FilePath } from '@/utils/constants';
import { SelectHelpers } from '@/components';

interface TestFixtures {
    detailsPage: DetailsPage;
}

const test = baseTest.extend<TestFixtures>({
    detailsPage: async ({ page }, use) => {
        const sourcePage = new DetailsPage(page);
        await use(sourcePage);
    },
});

const imgMap = {
    baseinfoDataCorrect: 'baseinfo-data-correct.png',
    baseinfoCompare: 'details-baseinfo-compare.png',
    coreOccupancyCorrect: 'core-occupancy-correct.png',
    coreOccupancyShowThroughput: 'core-occupancy-show-throughput.png',
    rooflineChart: 'roofline-chart.png',
    rooflineAdvice: 'roofline-advice.png',
    computeWorkloadChartCorrect: 'compute-workload-chart-correct.png',
    computeWorkloadChartCompare: 'compute-workload-chart-compare.png',
    computeWorkloadBlockIdChange: 'compute-workload-block-id-change.png',
    computeWorkloadAdvice: 'compute-workload-advice.png',
    computeWorkloadTable: 'compute-workload-table.png',
    computeWorkloadTableCompare: 'compute-workload-table-compare.png',
    memoryWorkloadChart: 'memory-workload-chart.png',
    memoryWorkloadChartCompare: 'memory-workload-chart-compare.png',
    memoryWorkloadAdvice: 'memory-workload-advice.png',
    memoryWorkloadTable: 'memory-workload-table.png',
    memoryWorkloadTableCompare: 'memory-workload-table-compare.png',
};
const inputMap = {
    coreOccupancyShowAsThroughput: 'Throughput',
    computeWorkloadBlockId: '1',
};

test.describe('Details', () => {
    test.beforeEach(async ({ page, detailsPage }, testInfo) => {
        await page.goto('/');
        await clearAllData(page);
        const filePath = testInfo.title.includes('roofline') ? FilePath.DETAILS_ROOFLINE : FilePath.DETAILS;
        await importData(page, filePath);
        await waitForAllParsed(page, detailsPage);
        const opType = detailsPage.detailsFrame.getByText('mix').first();
        await expect(opType).toBeVisible();
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });

    // 基本信息
    // 预期：基本信息文字、图表正确
    test('test_details_baseinfo_data_correct', async ({ detailsPage }) => {
        const { baseInfoContent } = detailsPage;
        await expect(baseInfoContent).toHaveScreenshot(imgMap.baseinfoDataCorrect);
    });

    // 核间负载分析
    // 预期：
    // 1、初始加载颜色数值正确
    // 2、下拉选项showAs修改后，数值、颜色正确
    test('test_details_core_occupancy_correct', async ({ page, detailsPage }) => {
        const { coreOccupancyContent, coreOccupancyShowAsSelector, detailsFrame } = detailsPage;
        await expect(coreOccupancyContent).toHaveScreenshot(imgMap.coreOccupancyCorrect);
        const showAsSelect = new SelectHelpers(page, coreOccupancyShowAsSelector, detailsFrame);
        await showAsSelect.open();
        await showAsSelect.selectOption(inputMap.coreOccupancyShowAsThroughput);
        await expect(coreOccupancyContent).toHaveScreenshot(imgMap.coreOccupancyShowThroughput);
    });
    // roofline瓶颈分析
    // 预期：导入数据，roofline图显示正确
    test('test_details_roofline', async ({ page, detailsPage }) => {
        const { rooflineChart } = detailsPage;
        // 等待 1s 图表动画完成
        await page.waitForTimeout(1000);
        await expect(rooflineChart).toHaveScreenshot(imgMap.rooflineChart);
    });
    // roofline瓶颈分析-瓶颈分析
    // 预期：roofline分析的瓶颈分析显示正确
    test('test_details_roofline_advice', async ({ detailsPage }) => {
        const { rooflineAdvice } = detailsPage;
        await expect(rooflineAdvice).toHaveScreenshot(imgMap.rooflineAdvice);
    });
    // 计算负载分析
    // 预期：
    // 1、柱状图显示正确
    // 2、下拉选项showAs修改后，数值、颜色正确
    test('test_details_compute_workload_correct', async ({ page, detailsPage }) => {
        const { ComputeWorkloadChart, computeWorkloadBlockIdSelector, detailsFrame } = detailsPage;

        // 此处切换 tab 是因为第一次快速切换至 detail 页面后， Compute Workload Analysis 图表可能不显示，该 bug 后续修复后删除该兼容逻辑
        const frameworkPage = new FrameworkPage(page);
        await frameworkPage.clickTab('timeline');
        await page.waitForTimeout(1000);
        await frameworkPage.clickTab('details');

        await expect(ComputeWorkloadChart).toHaveScreenshot(imgMap.computeWorkloadChartCorrect);
        const blockIdSelect = new SelectHelpers(page, computeWorkloadBlockIdSelector, detailsFrame);
        await blockIdSelect.open();
        await blockIdSelect.selectOption(inputMap.computeWorkloadBlockId);
        // 等待 1s 图表动画完成
        await page.waitForTimeout(1000);
        await expect(ComputeWorkloadChart).toHaveScreenshot(imgMap.computeWorkloadBlockIdChange);
    });

    // 计算负载分析-瓶颈分析结果
    // 预期：瓶颈分析结果正确
    test('test_details_compute_workload_advice', async ({ detailsPage }) => {
        const { computeWorkloadAdvice } = detailsPage;
        await expect(computeWorkloadAdvice).toHaveScreenshot(imgMap.computeWorkloadAdvice);
    });
    // 计算负载分析-计算负载信息表
    // 预期：瓶颈分析结果正确
    test('test_details_compute_workload_table', async ({ detailsPage }) => {
        const { computeWorkloadTable } = detailsPage;
        await expect(computeWorkloadTable).toHaveScreenshot(imgMap.computeWorkloadTable);
    });

    // 内存负载分析-热力图
    // 预期：
    // 1、导入数据成功
    // 2、热力图显示正确
    test('test_details_memory_workload_chart', async ({ detailsPage }) => {
        const { memoryWorkloadChart } = detailsPage;
        await expect(memoryWorkloadChart).toHaveScreenshot(imgMap.memoryWorkloadChart);
    });

    // 内存负载分析-瓶颈分析结果
    // 预期： 瓶颈分析结果正确
    test('test_details_memory_workload_advice', async ({ detailsPage }) => {
        const { memoryWorkloadAdvice } = detailsPage;
        await expect(memoryWorkloadAdvice).toHaveScreenshot(imgMap.memoryWorkloadAdvice);
    });
    // 内存负载分析-内存负载信息表
    // 预期：内存负载信息表正确
    test('test_details_memory_workload_table', async ({ detailsPage }) => {
        const { memoryWorkloadTable } = detailsPage;
        await expect(memoryWorkloadTable).toHaveScreenshot(imgMap.memoryWorkloadTable);
    });
});

test.describe('Details(Compare)', () => {
    test.beforeEach(async ({ page, detailsPage }) => {
        const { detailsFrame } = detailsPage;
        await page.goto('/');
        await clearAllData(page);
        await importData(page, FilePath.DETAILS);
        await waitForAllParsed(page, detailsPage);
        const opType = detailsFrame.getByText('mix').first();
        await expect(opType).toBeVisible();
        await importData(page, FilePath.DETAILS_ROOFLINE);
        await waitForAllParsed(page, detailsPage);
        await setCompare(page, detailsFrame, { baseline: FilePath.DETAILS, comparison: FilePath.DETAILS_ROOFLINE });
        await page.waitForTimeout(1000);
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });

    // 基本信息-对比
    // 预期：基本信息文字、图表正确
    test('test_details_baseinfo_compare', async ({ detailsPage }) => {
        const { baseInfoContent } = detailsPage;
        await expect(baseInfoContent).toHaveScreenshot(imgMap.baseinfoCompare);
    });

    // 计算负载分析-pipe utilization对比
    // 预期：柱状图正确
    test('test_details_compute_workload_chart_compare', async ({ detailsPage }) => {
        const { ComputeWorkloadChart } = detailsPage;
        await expect(ComputeWorkloadChart).toHaveScreenshot(imgMap.computeWorkloadChartCompare);
    });

    // 计算负载分析-信息表对比
    // 预期：表格信息正确
    test('test_details_compute_workload_table_compare', async ({ detailsPage }) => {
        const { computeWorkloadTable } = detailsPage;
        await expect(computeWorkloadTable).toHaveScreenshot(imgMap.computeWorkloadTableCompare);
    });

    // 内存负载分析-热力图对比
    // 预期：热力图显示正确
    test('test_details_memory_workload_chart_compare', async ({ detailsPage }) => {
        const { memoryWorkloadChart } = detailsPage;
        await expect(memoryWorkloadChart).toHaveScreenshot(imgMap.memoryWorkloadChartCompare);
    });

    // 内存负载分析-信息表对比
    // 预期：表格信息正确
    test('test_details_memory_workload_table_compare', async ({ detailsPage }) => {
        const { memoryWorkloadTable } = detailsPage;
        await expect(memoryWorkloadTable).toHaveScreenshot(imgMap.memoryWorkloadTableCompare);
    });
});
