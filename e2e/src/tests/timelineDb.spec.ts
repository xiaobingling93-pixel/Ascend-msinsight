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
    ws: Promise<WebSocket>;
}
const test = baseTest.extend<TestFixtures>({
    timelinePage: async ({ page }, use) => {
        const timelinePage = new TimelinePage(page);
        await use(timelinePage);
    },
    ws: async ({ page }, use) => {
        const ws = setupWebSocketListener(page);
        await use(ws);
    },
});
let parseClusterCompletedRes;
test.describe('Timeline(DB)', () => {
    test.beforeEach(async ({ page, timelinePage, ws }) => {
        const { timelineFrame } = timelinePage;
        await timelinePage.goto();
        await clearAllData(page);
        const allPagesSuccessRes = waitForResponse(await ws, (res) => res?.event === 'allPagesSuccess');
        parseClusterCompletedRes = waitForResponse(await ws, (res) => res?.event === 'parse/clusterCompleted');
        await importData(page, FilePath.DB_2025330);
        await allPagesSuccessRes;
        const secondLayerUnit = timelineFrame.locator('#main-container').getByText('Host', { exact: true });
        await expect(secondLayerUnit).toBeVisible();
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
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

    // 工具栏 - 算子搜索
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

    // 工具栏 - 算子搜索在泳道较深的位置时能显示在div中
    test('test_db_deepOperatorSearch_when_EnterOperatorName', async ({ page, timelinePage }) => {
        const { searchBtn, timelineFrame, openInWindows } = timelinePage;
        await searchBtn.click();
        const inputLocator = timelineFrame.locator('.insight-category-search-overlay input');
        const input = new InputHelpers(page, inputLocator, timelineFrame);
        await input.setValue('packaging/version.py(537): <lambda>');
        await input.press('Enter');
        await openInWindows.waitFor({ state: 'attached' });
        await page.mouse.move(0, 0);
        await page.waitForTimeout(1000);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('search-deep-operator.png', { maxDiffPixels: 200 });
    });

    // 工具栏 - 算子连线
    test('test_operatorLinkLine', async ({ page, timelinePage }) => {
        const { flowBtn, timelineFrame } = timelinePage;
        await timelineFrame.locator('div:nth-child(7) > .chart > div > .canvasContainer > .drawCanvas').click({
            position: {
                x: 84,
                y: 17,
            },
        });
        await timelineFrame.locator('div:nth-child(12) > .unit-info > .css-rdzxz6 > div > div > .insight-unit-fold').click();
        await timelineFrame.locator('div:nth-child(13) > .unit-info > .css-rdzxz6 > div > div > .insight-unit-fold').click();
        const LinkLineType = [
            'HostToDevice',
            'MsTx',
            'async_npu',
        ];
        for (const item of LinkLineType) {
            const LinkTypeCheckbox = timelineFrame.getByLabel(item);
            await flowBtn.click();
            await LinkTypeCheckbox.check();
            await flowBtn.click();
            await page.mouse.move(0, 0);
            await page.waitForTimeout(2000);
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`operator-link-line-${item}.png`, { maxDiffPixels: 100 });
            await flowBtn.click();
            await LinkTypeCheckbox.uncheck();
            await flowBtn.click();
        }
    });

    //todo: 工具栏 连线async_task_queue  && fwdbwd

    // 工具栏 - 泳道(card)过滤
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

    // 工具栏 - 泳道(unit)过滤
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
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('units-filter.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--右键点击通信算子跳转至通信页面
    test('test_db_redirectToCommunication_when_rightClickHCCLOperator', async ({ page, timelinePage, ws }) => {
        await parseClusterCompletedRes;
        test.setTimeout(30_000);
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
        await timelineFrame.getByText('Find in Communication').click({ force: true });
        const hcclChart = communicationFrame.locator('.panel-content').first();
        await hcclChart.waitFor({ state: 'visible' });

        await expect(hcclChart).toHaveScreenshot('redirect-to-communication.png', { maxDiffPixels: 100 });
    });

    // 同通信域泳道置顶
    test('test_db_same_communication_group', async ({ page, timelinePage, ws }) => {
        const { filterBtn, timelineFrame, selectFilterType,
            selectOptionFilterType, selectFilterContent } = timelinePage;
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
        await timelineFrame.locator('#unitWrapperScroller').click();
        await timelineFrame.locator('.insight-unit-fold > #Page-1 > [id="\\\\u9ED8\\\\u8BA4\\\\u9875\\\\u9762"] > [id="\\\\u7F16\\\\u7EC4\\\\u5907\\\\u4EFD"] > [id="list\\/item\\/Normal\\\\u5907\\\\u4EFD"] > #Group > [id="\\\\u77E9\\\\u5F62\\\\u5907\\\\u4EFD"]').first().click();
        await timelineFrame.locator('.insight-unit-fold').first().click();
        await timelineFrame.getByText('mp:Group group_name_41').click({
            button: 'right',
        });
        await timelineFrame.getByText('Pin (Same Group group_name_41)').click();
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('communication_group_pin.png', { maxDiffPixels: 100 });
        await timelineFrame.locator('#pinnedUnitWrapperScroller').getByText('localhost.localdomain4978604445055226587_0 0_Communication (HCCL)_mp:Group').click({
            button: 'right',
        });
        await timelineFrame.getByText('Unpin (Same Group').click();
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('communication_group_unpin.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--Show in Events View
    test('test_db_context_menu_click_ShowInEventsView', async ({ timelinePage, page }) => {
        const { timelineFrame, bottomPanel } = timelinePage;
        timelineFrame.locator('#unitWrapperScroller');
        await timelineFrame.locator('div:nth-child(12) > .unit-info > .css-rdzxz6 > div > div > .insight-unit-fold > #Page-1 > [id="\\\\u9ED8\\\\u8BA4\\\\u9875\\\\u9762"] > [id="\\\\u7F16\\\\u7EC4\\\\u5907\\\\u4EFD"] > [id="list\\/item\\/Normal\\\\u5907\\\\u4EFD"] > #Group > [id="\\\\u77E9\\\\u5F62\\\\u5907\\\\u4EFD"]').click();
        await timelineFrame.locator('div:nth-child(13) > .unit-info > .css-rdzxz6 > div > div > .insight-unit-fold > #Page-1 > [id="\\\\u9ED8\\\\u8BA4\\\\u9875\\\\u9762"] > [id="\\\\u7F16\\\\u7EC4\\\\u5907\\\\u4EFD"] > [id="list\\/item\\/Normal\\\\u5907\\\\u4EFD"] > #Group > [id="\\\\u77E9\\\\u5F62\\\\u5907\\\\u4EFD"]').click();
        await timelineFrame.locator('#unitWrapperScroller div').filter({ hasText: /^Stream 2$/ }).nth(3).click({
            button: 'right',
        });
        await timelineFrame.getByText('Show in Events View').click();
        await expect(bottomPanel).toHaveScreenshot('test-db-click-ShowInEventsView.png', { maxDiffPixels: 400 });
    });
});
