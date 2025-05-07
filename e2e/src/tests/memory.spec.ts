/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { expect, test as baseTest } from '@playwright/test';
import { FrameworkPage, MemoryPage, TimelinePage } from '@/page-object';
import { clearAllData, importData, setCompare, waitForWebSocketEvent } from '@/utils';
import { CheckboxHelpers, InputHelpers, SelectHelpers } from '@/components';
import { FilePath } from '@/utils/constants';

interface TestFixtures {
    memoryPage: MemoryPage;
}
const test = baseTest.extend<TestFixtures>({
    memoryPage: async ({ page }, use) => {
        const memoryPage = new MemoryPage(page);
        await use(memoryPage);
    },
});

const memoryImgMap = {
    loadPytorchSingleMachineMultiRankDataSuccess: 'memory-pytorch-single.png',
    filterPytorchSingleMachineMultiRankDataSuccess: 'memory-pytorch-single-filter.png',
    queryPytorchSingleMachineMultiRankDataSuccess: 'memory-pytorch-single-query.png',
    compareRankRes: 'memory-compare-rank.png',
    queryPytorchSingleMachineMultiRankDataOnlyShowAllocatedOrReleasedSuccess: 'memory-pytorch-interval-only-show.png',
    loadMindSporeDataSuccess: 'memory-mindspore-loaded.png',
    queryMindSporeDataSuccess: 'memory-mindspore-filter.png',
    loadMultiMachinesMultiRanksDataSuccess: 'memory-pytorch-multi-loaded.png',
    redirectToTimeline: 'memory-redirect-to-timeline.png',
    multiMachinesClickRank: 'memory-pytorch-multi-click-rank.png',
    dbToText: 'memory-pytorch-db-to-text.png',
    textToDb: 'memory-pytorch-text-to-db.png',
    compareRankDb: 'memory-compare-rank-db.png',
    queryCompareRankDb: 'memory-compare-rank-db-with-condition.png',
    pytorchTextGroupByComponent: 'memory-pytorch-group-by-component.png',
    sortRankGroupByComponent: 'memory-pytorch-sort-group-by-component.png',
    compareRankGroupByComponent: 'memory-pytorch-compare-group-by-component.png',
};

test.describe('Memory(Pytorch_SingleMachineMultiRankData)', () => {
    test.beforeEach(async ({ page, memoryPage }) => {
        const allCardParsedPromise = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        await memoryPage.goto();
        await clearAllData(page);
        await importData(page);
        await allCardParsedPromise;
    });

    // 【case】text非多机非对比memory界面加载
    test('loadMemoryPageSuccess_when_dataParseSuccess', async ({ page, memoryPage }) => {
        const { memoryFrame, rankIdSelector } = memoryPage;
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        await rankIdSelect.open();
        await rankIdSelect.selectOption('0');
        await page.mouse.move(0, 0);
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.loadPytorchSingleMachineMultiRankDataSuccess, {
            maxDiffPixels: 500,
        });
    });

    // 【case】memory顶部过滤条件改变界面加载
    test('change_filterCondition', async ({ page, memoryPage }) => {
        const { memoryFrame, rankIdSelector, groupIdSelector } = memoryPage;
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        const groupIdSelect = new SelectHelpers(page, groupIdSelector, memoryFrame);
        await rankIdSelect.open();
        await rankIdSelect.selectOption('4');
        await groupIdSelect.open();
        await groupIdSelect.selectOption('Stream');
        await page.mouse.move(0, 0);
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.filterPytorchSingleMachineMultiRankDataSuccess, {
            maxDiffPixels: 500,
        });
    });

    // 【case】memory底部表格条件查询后结果加载
    test('query_memoryDetailTable_by_tableFilterCondition', async ({ page, memoryPage }) => {
        const { memoryFrame, nameInputor, minSizeInputor, maxSizeInputor } = memoryPage;
        const nameInput = new InputHelpers(page, nameInputor, memoryFrame);
        const minSizeInput = new InputHelpers(page, minSizeInputor, memoryFrame);
        const maxSizeInput = new InputHelpers(page, maxSizeInputor, memoryFrame);
        await nameInput.setValue('aten::empty_strided');
        expect(await nameInput.expectValueToBe('aten::empty_strided'));
        expect(await minSizeInput.expectValueToBe('0'));
        expect(await maxSizeInput.expectValueToBe('503810'));
        const queryBtn = memoryFrame.getByTestId('query-btn');
        await queryBtn.waitFor({ state: 'visible' });
        await queryBtn.click();
        await page.mouse.move(0, 0);
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.queryPytorchSingleMachineMultiRankDataSuccess, {
            maxDiffPixels: 500,
        });
    });

    // 【case】memory底部表格条件重置后结果加载
    test('reset_memoryDetailTable_by_tableFilterCondition', async ({ page, memoryPage }) => {
        const { memoryFrame, maxSizeInputor } = memoryPage;
        const maxSizeInput = new InputHelpers(page, maxSizeInputor, memoryFrame);
        const resetBtn = memoryFrame.getByTestId('reset-btn');
        const queryBtn = memoryFrame.getByTestId('query-btn');

        const tableWrapper = memoryFrame.locator('.mi-page').locator('.panel-content').nth(1);

        await page.mouse.move(0, 0);
        await expect(tableWrapper).toHaveScreenshot('memory-pytorch-single-reset.png', {
            maxDiffPixels: 500,
        });

        await maxSizeInput.setValue('1000');
        await queryBtn.click();
        await page.mouse.move(0, 0);
        await expect(tableWrapper).toHaveScreenshot('memory-pytorch-single-reset-change.png', {
            maxDiffPixels: 500,
        });

        await resetBtn.click();
        await page.mouse.move(0, 0);
        await expect(tableWrapper).toHaveScreenshot('memory-pytorch-single-reset.png', {
            maxDiffPixels: 500,
        });
    });

    // 表格右键跳转至Timeline
    test('test_redirectToTimeline_when_rightClickTable', async ({ page, memoryPage }) => {
        const { memoryFrame } = memoryPage;
        const { fullPage } = new TimelinePage(page);
        await memoryFrame.getByRole('cell', { name: 'aten::empty_strided' }).first().click({
            button: 'right',
        });
        await memoryFrame.getByText('Find in Timeline').click();

        await page.mouse.move(0, 0);
        await expect(fullPage).toHaveScreenshot(memoryImgMap.redirectToTimeline, {
            maxDiffPixels: 500,
        });
    });

    // 对比数据
    test('memory_compare_rank_text', async ({ page, memoryPage }) => {
        const { memoryFrame } = memoryPage;
        await setCompare(page, memoryFrame);
        await memoryFrame.getByText('Difference').first().waitFor({ state: 'visible' });
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.compareRankRes, {
            maxDiffPixels: 500,
        });
        await page.waitForTimeout(2000); // 对比场景需要加延时，确保稳定
    });

    // 【case】memory中间调整区间，底部表格仅查看在选中时间区间分配或释放内存的数据时的结果展示
    test('query_memoryDetailTable_by_tableFilterConditionOnlyShowAllocatedOrReleasedWithinInterval', async ({ page, memoryPage }) => {
        const { memoryFrame, isOnlyShowAllocatedOrReleasedWithinIntervalChecker } = memoryPage;
        const isOnlyShowAllocatedOrReleasedWithinIntervalCheckbox = new CheckboxHelpers(page, isOnlyShowAllocatedOrReleasedWithinIntervalChecker, memoryFrame);

        const chart = memoryFrame.locator('.ant-spin-container > div > div:nth-child(2) > div:nth-child(1) > canvas');
        const chartInfo = await chart.boundingBox();
        if (!chartInfo) {
            return;
        }
        // 等待echarts加载完成才能框选
        await page.waitForTimeout(1000);
        const { x: startX, y: startY } = chartInfo;
        await page.mouse.move(startX + 300, startY + 200);
        await page.mouse.down();
        await page.mouse.move(startX + 400, startY + 200);
        await page.mouse.up();

        await isOnlyShowAllocatedOrReleasedWithinIntervalCheckbox.click();
        expect(await isOnlyShowAllocatedOrReleasedWithinIntervalCheckbox.isChecked()).toBe(true);
        const queryBtn = memoryFrame.getByTestId('query-btn');
        await queryBtn.waitFor({ state: 'visible' });
        await queryBtn.click();
        const spin = memoryFrame.locator('.panel-content .ant-spin-dot-spin');
        // 等待 loading 结束后端返回更新 totalNum
        await spin.waitFor({ state: 'detached' });
        await page.mouse.move(0, 0);

        const totalNumListItem = memoryFrame.locator('.ant-spin-container > ul > li.ant-pagination-total-text');
        expect(await totalNumListItem.innerText()).toBe('Total 229 items');

        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.queryPytorchSingleMachineMultiRankDataOnlyShowAllocatedOrReleasedSuccess, {
            maxDiffPixels: 500,
        });
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });
});

test.describe('Memory(MindSpore)', () => {
    test.describe.configure({ timeout: 120_000 });
    test.beforeEach(async ({ page, memoryPage }) => {
        const allCardParsedPromise = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        await memoryPage.goto();
        await clearAllData(page);
        await importData(page, FilePath.MIND_SPORE);
        await allCardParsedPromise;
    });

    // 测试图表加载完成
    test('test_staticChartDisplay_given_rank1', async ({ page, memoryPage }) => {
        const { memoryFrame, rankIdSelector } = memoryPage;
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        await rankIdSelect.open();
        await rankIdSelect.selectOption('1');

        // 等待 echarts 动画结束
        await page.waitForTimeout(1000);

        await page.mouse.move(0, 0);
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.loadMindSporeDataSuccess, {
            maxDiffPixels: 500,
        });
    });

    // 测试鼠标滚动、框选、查询
    test('test_staticChartDisplay_with_conditions', async ({ page, memoryPage }) => {
        const { memoryFrame, nameInputor, minSizeInputor, rankIdSelector } = memoryPage;
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        await rankIdSelect.open();
        await rankIdSelect.selectOption('0');
        // 等待echarts加载完成再操作
        await page.waitForTimeout(1000);
        // 滚动到最下方，让graph折线图和表格显示出来
        await page.mouse.wheel(0, 800);
        await page.waitForTimeout(1000);
        // 框选
        const graphChart = memoryFrame.locator('canvas').nth(1);
        expect(graphChart).not.toBeNull();
        const chartInfo = await graphChart.boundingBox();
        const { x: startX, y: startY } = chartInfo;
        await page.mouse.move(startX + 300, startY + 200);
        await page.mouse.down();
        await page.mouse.move(startX + 400, startY + 200);
        await page.mouse.up();
        const nameInput = new InputHelpers(page, nameInputor, memoryFrame);
        const minSizeInput = new InputHelpers(page, minSizeInputor, memoryFrame);
        expect(await nameInput.expectValueToBe(''));
        expect(await minSizeInput.expectValueToBe('0'));
        await nameInput.setValue('attention');
        await minSizeInput.setValue('10000');
        const queryBtn = memoryFrame.getByTestId('query-btn');
        await queryBtn.click();
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.queryMindSporeDataSuccess, {
            maxDiffPixels: 500,
        });
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });
});

test.describe('Memory(Pytorch_MultiMachinesMultiRanksData)', () => {
    test.beforeEach(async ({ page, memoryPage }) => {
        const allCardParsedPromise = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        await memoryPage.goto();
        await clearAllData(page);
        await importData(page, FilePath.MULTI_MACHINES);
        await allCardParsedPromise;
    });

    // 多机多卡数据界面正常加载，切换卡序号正常显示
    test('test_pageDisplay_when_change_rank', async ({ page, memoryPage }) => {
        const { memoryFrame, rankIdSelector } = memoryPage;
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        await rankIdSelect.open();
        await rankIdSelect.selectOption('1');

        // 等待 echarts 动画结束
        await page.waitForTimeout(1000);

        await page.mouse.move(0, 0);
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.loadMultiMachinesMultiRanksDataSuccess, {
            maxDiffPixels: 500,
        });
    });

    // 多机多卡数据点击目录时会自动切换到点击的卡
    test('test_pageDisplay_when_click_rank', async ({ page, memoryPage }) => {
        const frameworkPage = new FrameworkPage(page);
        const { memoryFrame, hostSelector, rankIdSelector } = memoryPage;
        const hostSelect = new SelectHelpers(page, hostSelector, memoryFrame);

        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        const dbHost0Rank1 = frameworkPage.getRankLocator(FilePath.DB_HOST_0_RANK_1);
        await dbHost0Rank1.click();
        // 等待 echarts 动画结束
        await page.waitForTimeout(1000);
        expect(await hostSelect.getValue()).toBe('node18899436934890168541_0');
        expect(await rankIdSelect.getValue()).toBe('1');

        const dbHost1Rank1 = frameworkPage.getRankLocator(FilePath.DB_HOST_1_RANK_1);
        await dbHost1Rank1.click();
        await page.waitForTimeout(1000);
        expect(await hostSelect.getValue()).toBe('ubuntu22044973785946912235777_0');
        expect(await rankIdSelect.getValue()).toBe('9');
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.multiMachinesClickRank, {
            maxDiffPixels: 500,
        });
    });

    // 多机多卡数据切换机器时自动选中首张卡
    test('test_pageDisplay_when_change_host', async ({ page, memoryPage }) => {
        const { memoryFrame, hostSelector, rankIdSelector } = memoryPage;
        const hostSelect = new SelectHelpers(page, hostSelector, memoryFrame);
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        await hostSelect.open();
        await hostSelect.selectOption('ubuntu22044973785946912235777_0');
        expect(await rankIdSelect.getValue()).toBe('8');
        await hostSelect.open();
        await hostSelect.selectOption('node18899436934890168541_0');
        expect(await rankIdSelect.getValue()).toBe('0');
    });

    // 多机多卡db类型数据比对场景
    test('test_pageDisplay_compare_rank_db', async ({ page, memoryPage }) => {
        const { minSizeInputor, maxSizeInputor, memoryFrame } = memoryPage;
        await setCompare(page, memoryFrame, { baseline: FilePath.DB_HOST_0_RANK_0, comparison: FilePath.DB_HOST_1_RANK_0 });
        await memoryFrame.getByText('Difference').first().waitFor({ state: 'visible' });
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.compareRankDb, {
            maxDiffPixels: 500,
        });
        const minSizeInput = new InputHelpers(page, minSizeInputor, memoryFrame);
        const maxSizeInput = new InputHelpers(page, maxSizeInputor, memoryFrame);
        const queryBtn = memoryFrame.getByTestId('query-btn');
        const resetBtn = memoryFrame.getByTestId('reset-btn');
        await minSizeInput.expectValueToBe('-243840');
        await maxSizeInput.expectValueToBe('243840');
        await minSizeInput.setValue('0');
        await queryBtn.click();
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.queryCompareRankDb, {
            maxDiffPixels: 500,
        });
        await resetBtn.click();
        await minSizeInput.expectValueToBe('-243840');
        await page.waitForTimeout(2000); // 对比场景需要加延时，确保稳定
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });
});

test.describe('Memory(Pytorch_SwitchProject)', () => {
    test.beforeEach(async ({ page, memoryPage }) => {
        const allCardParsedPromise = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        await memoryPage.goto();
        await clearAllData(page);
        await importData(page);
        await allCardParsedPromise;
        await importData(page, FilePath.MULTI_MACHINES);
        await allCardParsedPromise;
    });

    // 测试从多机多卡db类型数据切换到单机多卡text类型数据
    test('test_pageDisplay_when_db_to_text', async ({ page, memoryPage }) => {
        const frameworkPage = new FrameworkPage(page);
        const dbRank0 = frameworkPage.getRankLocator(FilePath.DB_HOST_0_RANK_0);
        await dbRank0.click();
        const textRank1 = frameworkPage.getRankLocator(FilePath.TEXT_RANK_1);
        await textRank1.click();
        const { memoryFrame, hostSelector, rankIdSelector } = memoryPage;
        await hostSelector.waitFor({ state: 'detached' });
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        const selectedText = await rankIdSelect.getValue();
        expect(selectedText).toBe('1');
        // 等待 echarts 动画结束
        await page.waitForTimeout(1000);
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.dbToText, {
            maxDiffPixels: 500,
        });
    });

    // 测试从单机多卡text类型数据切换到多机多卡db类型数据
    test('test_pageDisplay_when_text_to_db', async ({ page, memoryPage }) => {
        const frameworkPage = new FrameworkPage(page);
        const textRank0 = frameworkPage.getRankLocator(FilePath.TEXT_RANK_0);
        await textRank0.click();
        const dbRank1 = frameworkPage.getRankLocator(FilePath.DB_HOST_0_RANK_1);
        await dbRank1.click();
        const { memoryFrame, hostSelector, rankIdSelector } = memoryPage;
        await hostSelector.waitFor({ state: 'attached' });
        const hostSelect = new SelectHelpers(page, hostSelector, memoryFrame);
        const hostText = await hostSelect.getValue();
        expect(hostText).toBe('node18899436934890168541_0');
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        const selectedText = await rankIdSelect.getValue();
        expect(selectedText).toBe('1');
        // 等待 echarts 动画结束
        await page.waitForTimeout(2000);
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.textToDb, {
            maxDiffPixels: 500,
        });
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });
});

test.describe('Memory(Pytorch_Group_By_Component', () => {
    test.beforeEach(async ({ page, memoryPage }) => {
        const allCardParsedPromise = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        await memoryPage.goto();
        await clearAllData(page);
        await importData(page);
        await allCardParsedPromise;
        const { memoryFrame, groupIdSelector } = memoryPage;
        const groupIdSelect = new SelectHelpers(page, groupIdSelector, memoryFrame);
        await groupIdSelect.open();
        await groupIdSelect.selectOption('Component');
        await page.waitForTimeout(1000);
    });

    // 组件级内存展示测试
    test('test_pageDisplay_group_by_Component', async({ page, memoryPage }) => {
        const { memoryFrame, rankIdSelector } = memoryPage;
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, memoryFrame);
        await rankIdSelect.open();
        await rankIdSelect.selectOption('1');
        await page.waitForTimeout(1000);
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.pytorchTextGroupByComponent, {
            maxDiffPixels: 500,
        });
    });

    // 组件级内存排序测试
    test('test_sort_group_by_Component', async({ memoryPage }) => {
        const { memoryFrame } = memoryPage;
        await memoryFrame.getByRole('table').getByText('Peak Memory Reserved(MB)').click();
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.sortRankGroupByComponent, {
            maxDiffPixels: 500,
        });
    });

    // 组件级内存比对场景测试
    test('test_compare_group_by_Component', async ({ page, memoryPage }) => {
        const { memoryFrame } = memoryPage;
        await setCompare(page, memoryFrame);
        await memoryFrame.getByText('Difference').first().waitFor({ state: 'visible' });
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.compareRankGroupByComponent, {
            maxDiffPixels: 500,
        });
        await memoryFrame.getByRole('row', { name: 'APP Difference 0 15.540 See' }).getByRole('button').click();
        await expect(memoryFrame.locator('tbody')).toContainText('104.2');
        await memoryFrame.getByRole('row', { name: 'HCCL Difference -400.004 15.' }).getByRole('button').click();
        await expect(memoryFrame.locator('tbody')).toContainText('1600.046875');
        await page.waitForTimeout(2000); // 对比场景需要加延时，确保稳定
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });
});
