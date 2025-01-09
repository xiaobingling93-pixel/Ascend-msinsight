/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { expect, test as baseTest } from '@playwright/test';
import { MemoryPage, TimelinePage } from './page-object';
import { clearAllData, importData, setCompare, waitForWebSocketEvent } from './utils';
import { CheckboxHelpers, InputHelpers, SelectHelpers } from './components';
import { FilePath } from './utils/constants';

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
    resetPytorchSingleMachineMultiRankDataSuccess: 'memory-pytorch-single-reset.png',
    compareRankRes: 'memory-compare-rank.png',
    queryPytorchSingleMachineMultiRankDataOnlyShowAllocatedOrReleasedSuccess: 'memory-pytorch-interval-only-show.png',
    loadMindSporeDataSuccess: 'memory-mindspore-loaded.png',
    loadMultiMachinesMultiRanksDataSuccess: 'memory-pytorch-multi-loaded.png',
    redirectToTimeline: 'memory-redirect-to-timeline.png',
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
        expect(await maxSizeInput.expectValueToBe('1000000'));
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
        const { memoryFrame, nameInputor, minSizeInputor, maxSizeInputor } = memoryPage;
        const nameInput = new InputHelpers(page, nameInputor, memoryFrame);
        const minSizeInput = new InputHelpers(page, minSizeInputor, memoryFrame);
        const maxSizeInput = new InputHelpers(page, maxSizeInputor, memoryFrame);
        expect(await nameInput.expectValueToBe(''));
        expect(await minSizeInput.expectValueToBe('0'));
        expect(await maxSizeInput.expectValueToBe('1000000'));
        const queryBtn = memoryFrame.getByTestId('reset-btn');
        await queryBtn.waitFor({ state: 'visible' });
        await queryBtn.click();
        await page.mouse.move(0, 0);
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.resetPytorchSingleMachineMultiRankDataSuccess, {
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
    test('memory_compare_rank', async ({ page, memoryPage }) => {
        const { memoryFrame } = memoryPage;
        await setCompare(page, memoryFrame);
        await memoryFrame.getByText('Difference').first().waitFor({ state: 'visible' });
        await expect(memoryFrame.locator('.mi-page')).toHaveScreenshot(memoryImgMap.compareRankRes, {
            maxDiffPixels: 500,
        });
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

    // 多机多卡数据界面正常加载
    test('test_pageDisplay_when_changeMachines', async ({ page, memoryPage }) => {
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

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });
});
