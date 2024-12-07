/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import {expect, test as baseTest} from '@playwright/test';
import {DetailsPage} from './page-object';
import {importData} from './utils';
import {FilePath} from './utils/constants';
import {SelectHelpers} from './components';

interface TestFixtures {
    detailsPage: DetailsPage;
}

const test = baseTest.extend<TestFixtures>({
    detailsPage: async ({page}, use) => {
        const sourcePage = new DetailsPage(page);
        await use(sourcePage);
    },
});

const imgMap = {
    baseinfoDataCorrect: 'baseinfo-data-correct.png',
    coreOccupancyCorrect: 'core-occupancy-correct.png',
    coreOccupancyShowThroughput: 'core-occupancy-show-throughput.png',
    computeWorkloadChartCorrect: 'compute-workload-chart-correct.png',
    computeWorkloadBlockIdChange: 'compute-workload-block-id-change.png',
};
const inputMap = {
    coreOccupancyShowAsThroughput: 'Throughput',
    computeWorkloadBlockId: '1',
};

test.describe('Details', () => {
    test.beforeEach(async ({page, detailsPage}) => {
        await page.goto('/');
        await importData(page, FilePath.DETAILS);
        await detailsPage.goto();
        const opType = detailsPage.detailsFrame.getByText('mix').first();
        await expect(opType).toBeVisible();
    });

    // 基本信息
    // 预期：基本信息文字、图表正确
    test('test_details_baseinfo_data_correct', async ({detailsPage}) => {
        const {baseInfoContent} = detailsPage;
        await expect(baseInfoContent).toHaveScreenshot(imgMap.baseinfoDataCorrect);
    });

    // 核间负载分析
    // 预期：
    // 1、初始加载颜色数值正确
    // 2、下拉选项showAs修改后，数值、颜色正确
    test('test_details_core_occupancy_correct', async ({page, detailsPage}) => {
        const {coreOccupancyContent, coreOccupancyShowAsSelector, detailsFrame} = detailsPage;
        await expect(coreOccupancyContent).toHaveScreenshot(imgMap.coreOccupancyCorrect);
        const showAsSelect = new SelectHelpers(page, coreOccupancyShowAsSelector, detailsFrame);
        await showAsSelect.open();
        await showAsSelect.selectOption(inputMap.coreOccupancyShowAsThroughput);
        await expect(coreOccupancyContent).toHaveScreenshot(imgMap.coreOccupancyShowThroughput);
    });
    // 计算负载分析
    // 预期：
    // 1、柱状图显示正确
    // 2、下拉选项showAs修改后，数值、颜色正确
    test('test_details_compute_workload_correct', async ({page, detailsPage}) => {
        const {ComputeWorkloadChart, computeWorkloadBlockIdSelector, detailsFrame} = detailsPage;
        await expect(ComputeWorkloadChart).toHaveScreenshot(imgMap.computeWorkloadChartCorrect);
        const blockIdSelect = new SelectHelpers(page, computeWorkloadBlockIdSelector, detailsFrame);
        await blockIdSelect.open();
        await blockIdSelect.selectOption(inputMap.computeWorkloadBlockId);
        await expect(ComputeWorkloadChart).toHaveScreenshot(imgMap.computeWorkloadBlockIdChange);
    });
});
