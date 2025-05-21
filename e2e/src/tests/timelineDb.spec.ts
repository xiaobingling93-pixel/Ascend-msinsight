/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { expect, test as baseTest, WebSocket } from '@playwright/test';
import { CommunicationPage, SystemView, TimelinePage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener, waitForResponse } from '@/utils';
import { FilePath } from '@/utils/constants';
import { InputHelpers, SelectHelpers } from '@/components';

interface TestFixtures {
    timelinePage: TimelinePage;
}
const test = baseTest.extend<TestFixtures>({
    timelinePage: async ({ page }, use) => {
        const timelinePage = new TimelinePage(page);
        await use(timelinePage);
    },
});
let ws: Promise<WebSocket>;
test.describe('Timeline', () => {
    test.beforeEach(async ({ page, timelinePage }) => {
        ws = setupWebSocketListener(page);
        const { timelineFrame } = timelinePage;
        await timelinePage.goto();
        await clearAllData(page);
        await importData(page, FilePath.DB_2025330);
        const secondLayerUnit = timelineFrame.locator('#main-container').getByText('Host', { exact: true });
        await expect(secondLayerUnit).toBeVisible();
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });

    // System View - Stats System View 数据展示
    test('test_db_StatsSystemViewDataDisplay_in_SystemView', async ({ page, timelinePage }) => {
        const { bottomPanel, timelineFrame } = timelinePage;
        const systemView = new SystemView(page);
        await systemView.goto();

        const statsSystemViewOptions = [
            'Overall Metrics',
            'Python API Summary',
            'CANN API Summary',
            'Ascend HardWare Task Summary',
            'Communication Summary',
            'Overlap Analysis',
            'Kernel Details',
        ];
        await page.waitForTimeout(2500);
        await expect(bottomPanel).toHaveScreenshot('StatsSystemView-Overall-Metrics.png', { maxDiffPixels: 400 });

        for (const item of statsSystemViewOptions) {
            const option = timelineFrame.getByText(item, { exact: true });
            await option.click();
            await timelineFrame.locator('.ant-spin').waitFor({ state: 'hidden' });
            await expect(bottomPanel).toHaveScreenshot(`StatsSystemView-${item}.png`, { maxDiffPixels: 400 });
        }
    });

    // System View - Expert System View 数据展示
    test('test_db_ExpertSystemViewDataDisplay_in_SystemView', async ({ page, timelinePage }) => {
        const { bottomPanel, timelineFrame } = timelinePage;
        const systemViewPage = new SystemView(page);
        const systemViewSelector = new SelectHelpers(page, systemViewPage.selectSystemView, timelineFrame);
        await systemViewPage.goto();

        await systemViewSelector.open();
        await systemViewSelector.selectOption('Expert System View');

        const expertSystemViewOptions = ['Expert Analysis', 'Affinity API', 'Affinity Optimizer',
            'AICPU Operators', 'ACLNN Operators', 'Operators Fusion'];

        for (const item of expertSystemViewOptions) {
            const option = timelineFrame.getByText(item, { exact: true });
            await option.click();
            await timelineFrame.locator('.ant-spin').waitFor({ state: 'hidden' });
            await expect(bottomPanel).toHaveScreenshot(`ExpertSystemView-${item}.png`, { maxDiffPixels: 400 });
        }
    });

    // 算子搜索
    test('test_db_operatorSearch_when_EnterOperatorName', async ({ page, timelinePage }) => {
        const { searchBtn, timelineFrame, openInWindows } = timelinePage;
        await searchBtn.click();
        const inputLocator = timelineFrame.locator('.insight-category-search-overlay input');
        const input = new InputHelpers(page, inputLocator, timelineFrame);
        await input.setValue('CtxGetOverflowAddr');
        await input.press('Enter');
        await openInWindows.waitFor({ state: 'attached' });
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('search-operator.png', { maxDiffPixels: 200 });
    });

    // 算子搜索在泳道较深的位置时能显示在div中
    test('test_db_deepOperatorSearch_when_EnterOperatorName', async ({ page, timelinePage }) => {
        const { searchBtn, timelineFrame, openInWindows } = timelinePage;
        await searchBtn.click();
        const inputLocator = timelineFrame.locator('.insight-category-search-overlay input');
        const input = new InputHelpers(page, inputLocator, timelineFrame);
        await input.setValue('packaging/version.py(537): <lambda>');
        await input.press('Enter');
        await openInWindows.waitFor({ state: 'attached' });
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('search-deep-operator.png', { maxDiffPixels: 200 });
    });

    // 泳道(card)过滤
    test('test_db_cardFilter', async ({ page, timelinePage }) => {
        const { filterBtn, timelineFrame, selectFilterType, selectOptionFilterType, selectFilterContent } = timelinePage;
        const filterTypeSelector = new SelectHelpers(page, selectFilterType, timelineFrame);
        const filterContentSelector = new SelectHelpers(page, selectFilterContent, timelineFrame);

        await filterBtn.click();
        await page.mouse.move(0, 0);

        await filterTypeSelector.open();
        // 由于该 select 框下拉选项是自定义节点，不能使用 SelectHelpers 的 selectOption 方法取值
        const option = selectOptionFilterType.getByText('Card Filter');
        await option.click();
        await page.mouse.move(0, 0);

        await filterContentSelector.open();
        await filterContentSelector.selectOption('3');
        await filterBtn.click();
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('card-filter.png', { maxDiffPixels: 200 });
    });

    // 泳道(unit)过滤
    test('test_db_unitFilter', async ({ page, timelinePage }) => {
        const { filterBtn, timelineFrame, selectFilterType, selectOptionFilterType, selectFilterContent } = timelinePage;
        const filterTypeSelector = new SelectHelpers(page, selectFilterType, timelineFrame);
        const filterContentSelector = new SelectHelpers(page, selectFilterContent, timelineFrame);

        await filterBtn.click();
        await page.mouse.move(0, 0);

        await filterTypeSelector.open();
        // 由于该 select 框下拉选项是自定义节点，不能使用 SelectHelpers 的 selectOption 方法取值
        const option = selectOptionFilterType.getByText('Units Filter');
        await option.click();

        await page.mouse.move(0, 0);
        await filterContentSelector.open();
        await filterContentSelector.setValue('Ascend Hardware');
        await filterContentSelector.selectOption('Ascend Hardware');
        await filterBtn.click();
        await page.mouse.move(0, 0);
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('units-filter.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--右键点击通信算子跳转至通信页面
    test('test_db_redirectToCommunication_when_rightClickHCCLOperator', async ({ page, timelinePage }) => {
        test.setTimeout(200_000);
        const { communicationFrame } = new CommunicationPage(page);
        const { filterBtn, timelineFrame, selectFilterType,
            selectOptionFilterType, selectFilterContent, unitWrapperScroller,
            searchBtn, openInWindows } = timelinePage;
        const filterTypeSelector = new SelectHelpers(page, selectFilterType, timelineFrame);
        const filterContentSelector = new SelectHelpers(page, selectFilterContent, timelineFrame);

        await filterBtn.click();
        await page.mouse.move(0, 0);

        await filterTypeSelector.open();
        // 由于该 select 框下拉选项是自定义节点，不能使用 SelectHelpers 的 selectOption 方法取值
        const option = selectOptionFilterType.getByText('Units Filter');
        await option.click();

        await page.mouse.move(0, 0);
        await filterContentSelector.open();
        await filterContentSelector.setValue('Communication');
        await filterContentSelector.selectOption('Communication');
        await filterBtn.click();
        await page.mouse.move(0, 0);
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();

        await unitWrapperScroller.getByText('Communication').click();
        await searchBtn.click();
        const inputLocator = timelineFrame.locator('.insight-category-search-overlay input').nth(2);
        const input = new InputHelpers(page, inputLocator, timelineFrame);
        await input.setValue('hcom_allGather__491_201_1');
        await input.press('Enter');
        await openInWindows.waitFor({ state: 'attached' });
        await timelineFrame.locator('.chart-selected > div > .canvasContainer > .drawCanvas').click({
            position: {
                x: 659,
                y: 5,
            },
        });
        await timelineFrame.locator('.chart-selected > div > .canvasContainer > .drawCanvas').click({
            button: 'right',
            position: {
                x: 659,
                y: 5,
            },
        });
        await waitForResponse(await ws, (res) => res?.event === 'parse/clusterCompleted');
        await timelineFrame.getByText('Find in Communication').click({ force: true });
        const hcclChart = communicationFrame.locator('.panel-content').first();
        await hcclChart.waitFor({ state: 'visible' });

        await expect(hcclChart).toHaveScreenshot('redirect-to-communication.png', { maxDiffPixels: 100 });
    });
});