/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { test as baseTest, expect } from '@playwright/test';
import { TimelinePage, SystemView } from './page-object';
import { clearAllData, importData } from './utils';
import { InputHelpers, SelectHelpers } from './components';
import { FilePath } from './utils/constants';

interface TestFixtures {
    timelinePage: TimelinePage;
}
const test = baseTest.extend<TestFixtures>({
    timelinePage: async ({ page }, use) => {
        const timelinePage = new TimelinePage(page);
        await use(timelinePage);
    },
});

test.describe('Timeline', () => {
    test.beforeEach(async ({ page, timelinePage }) => {
        const { timelineFrame } = timelinePage;
        await timelinePage.goto();
        await importData(page);
        const secondLayerUnit = timelineFrame.locator('#main-container').getByText('Python (2045554)');
        await expect(secondLayerUnit).toBeVisible();
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });

    // 泳道展开收缩
    test('test_unitsExpandAndCollapse_when_click', async ({ timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const secondLayerUnit = timelineFrame.locator('#main-container').getByText('Python (2045554)');
        const firstUnitInfo = timelineFrame.locator('.unit-info').first();
        await firstUnitInfo.click();
        await expect(secondLayerUnit).toBeHidden();
        await firstUnitInfo.click();
        await expect(secondLayerUnit).toBeVisible();
    });

    // 泳道置顶
    test('test_unitsPinToTopAndUnpin', async ({ timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const firstUnitInfo = timelineFrame.locator('.unit-info').first();
        await firstUnitInfo.hover();
        const pinBtn = timelineFrame.getByTestId('pin-btn');
        await pinBtn.waitFor({ state: 'visible' });
        await pinBtn.click();
        const firstLayerUnit = timelineFrame.locator('#main-container').getByText('0', { exact: true });
        const secondLayerUnit = timelineFrame.locator('#main-container').getByText('Python (2045554)');
        expect(await firstLayerUnit.count()).toBe(2);
        expect(await secondLayerUnit.count()).toBe(2);

        await pinBtn.first().click();
        expect(await firstLayerUnit.count()).toBe(1);
        expect(await secondLayerUnit.count()).toBe(1);
    });

    // 偏移量设置
    test('test_offsetConfig', async ({ page, timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const firstUnitInfo = timelineFrame.locator('.unit-info').first();
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();
        await firstUnitInfo.hover({ force: true });
        // 这里需要优化，改为当图表渲染完成后再继续执行
        await page.waitForTimeout(1500);
        const offsetBtn = timelineFrame.getByTestId('offset-btn').first();
        await offsetBtn.click();
        const offsetInput = timelineFrame.getByRole('tooltip', { name: /Timestamp Offset/i }).getByRole('textbox');
        await offsetInput.fill('300000000');
        await offsetInput.press('Enter');
        await offsetBtn.click();
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('unit-offset.png', { maxDiffPixels: 100 });
    });

    // 点击算子展示算子详情
    test('test_showSliceDetail_when_clickOperator', async ({ page, timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();
        // 这里需要优化，改为当图表渲染完成后再继续执行
        await page.waitForTimeout(1000);
        // 点击算子
        await timelineFrame
            .locator('.chart > div > .canvasContainer > .drawCanvas')
            .first()
            .click({
                position: {
                    x: 79,
                    y: 9,
                },
            });
        await expect(timelineFrame.getByText('Title')).toBeVisible();
    });

    // 框选泳道展示算子列表
    test('test_showSliceList_when_selectUnitsRange', async ({ page, timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();
        const chart = timelineFrame.locator('.chart-selected > div > .canvasContainer > .drawCanvas');
        const chartInfo = await chart.boundingBox();
        if (!chartInfo) {
            return;
        }
        const { x: startX, y: startY } = chartInfo;

        await page.mouse.move(startX + 50, startY + 50);
        await page.mouse.down();
        await page.mouse.move(startX + 200, startX - 200);
        await page.mouse.up();
        await expect(timelineFrame.getByText('Wall Duration', { exact: true })).toBeVisible();
        await expect(timelineFrame.getByText('Self Time')).toBeVisible();
        await expect(timelineFrame.getByText('Average Wall Duration')).toBeVisible();
        const rows = await timelineFrame.locator('.ant-table-row').count();
        expect(rows).toBeGreaterThan(0);
    });
    // System View - Stats System View 数据展示
    test('test_StatsSystemViewDataDisplay_in_SystemView', async ({ page, timelinePage }) => {
        const { bottomPanel, timelineFrame } = timelinePage;
        const systemView = new SystemView(page);
        await systemView.goto();

        const statsSystemViewOptions = [
            'Python API Summary',
            'CANN API Summary',
            'Ascend HardWare Task Summary',
            'HCCL Summary',
            'Overlap Analysis',
            'Kernel Details',
        ];

        for (let item of statsSystemViewOptions) {
            const option = timelineFrame.getByText(item, { exact: true });
            await option.click();
            await timelineFrame.locator('.ant-spin').waitFor({ state: 'visible' });
            await timelineFrame.locator('.ant-spin').waitFor({ state: 'hidden' });
            await expect(bottomPanel).toHaveScreenshot(`StatsSystemView-${item}.png`, { maxDiffPixels: 100 });
        }
    });

    // System View - Expert System View 数据展示
    test('test_ExpertSystemViewDataDisplay_in_SystemView', async ({ page, timelinePage }) => {
        const { bottomPanel, timelineFrame } = timelinePage;
        const systemViewPage = new SystemView(page);
        const systemViewSelector = new SelectHelpers(page, systemViewPage.selectSystemView, timelineFrame);
        await systemViewPage.goto();

        await systemViewSelector.open();
        await systemViewSelector.selectOption('Expert System View');

        const expertSystemViewOptions = ['Affinity API', 'Affinity Optimizer', 'AICPU Operators', 'ACLNN Operators', 'Operators Fusion'];

        for (let item of expertSystemViewOptions) {
            const option = timelineFrame.getByText(item, { exact: true });
            await option.click();
            await timelineFrame.locator('.ant-spin').waitFor({ state: 'visible' });
            await timelineFrame.locator('.ant-spin').waitFor({ state: 'hidden' });
            await expect(bottomPanel).toHaveScreenshot(`ExpertSystemView-${item}.png`, { maxDiffPixels: 100 });
        }
    });

    // 算子搜索
    test('test_operatorSearch_when_EnterOperatorName', async ({ page, timelinePage }) => {
        const { searchBtn, timelineFrame } = timelinePage;
        await searchBtn.click();
        const inputLocator = timelineFrame.locator('.insight-category-search-overlay input');
        const input = new InputHelpers(page, inputLocator, timelineFrame);
        await input.setValue('add');
        await input.press('Enter');
        await page.mouse.move(0, 0);
        await page.waitForTimeout(1000);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('search-operator.png', { maxDiffPixels: 100 });
    });

    // 泳道(card)过滤
    test('test_cardFilter', async ({ page, timelinePage }) => {
        const { filterBtn, timelineFrame, selectFilterType, selectOptionFilterType, selectFilterContent } = timelinePage;
        const filterTypeSelector = new SelectHelpers(page, selectFilterType, timelineFrame);
        const filterContentSelector = new SelectHelpers(page, selectFilterContent, timelineFrame);

        await filterBtn.click();
        await page.mouse.move(0, 0);

        await filterTypeSelector.open();
        // 由于该 select 框下拉选项是自定义节点，不能使用 SelectHelpers 的 selectOption 方法取值
        const option = selectOptionFilterType.getByText('Card Filter');
        await option.click();

        await filterContentSelector.open();
        await filterContentSelector.selectOption('0');
        await filterBtn.click();
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('card-filter.png', { maxDiffPixels: 100 });
    });

    // 泳道(unit)过滤
    test('test_unitFilter', async ({ page, timelinePage }) => {
        const { filterBtn, timelineFrame, selectFilterType, selectOptionFilterType, selectFilterContent } = timelinePage;
        const filterTypeSelector = new SelectHelpers(page, selectFilterType, timelineFrame);
        const filterContentSelector = new SelectHelpers(page, selectFilterContent, timelineFrame);

        await filterBtn.click();
        await page.mouse.move(0, 0);

        await filterTypeSelector.open();
        // 由于该 select 框下拉选项是自定义节点，不能使用 SelectHelpers 的 selectOption 方法取值
        const option = selectOptionFilterType.getByText('Units Filter');
        await option.click();

        await filterContentSelector.open();
        await filterContentSelector.selectOption('Ascend Hardware ');
        await filterBtn.click();
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('units-filter.png', { maxDiffPixels: 100 });
    });

    // 算子连线
    test('test_operatorLinkLine', async ({ page, timelinePage }) => {
        const { flowBtn, timelineFrame } = timelinePage;
        const hostToDeviceCheckbox = timelineFrame.getByLabel('HostToDevice');
        const hcclUnit = timelineFrame.locator('#unitWrapperScroller').getByText('HCCL (2094647744)');

        await flowBtn.click();
        await hostToDeviceCheckbox.check();
        await flowBtn.click();
        await hcclUnit.click();
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('operator-link-line.png', { maxDiffPixels: 100 });
    });

    // 缩放按钮、重置按钮
    test('test_zoomAndResetUnit', async ({ page, timelinePage }) => {
        const { resetBtn, zoomInBtn, zoomOutBtn, timelineFrame } = timelinePage;

        const mainContainer = timelineFrame.locator('#main-container');
        const secondLayerUnit = timelineFrame.locator('#main-container').getByText('Python (2045554)');
        await secondLayerUnit.click();

        for (let i = 0; i < 5; i++) {
            await zoomOutBtn.click();
            if (i < 4) {
                await page.waitForTimeout(200);
            }
        }
        await page.mouse.move(0, 0);
        await expect(mainContainer).toHaveScreenshot('zoom-out-unit.png', { maxDiffPixels: 100 });

        for (let i = 0; i < 3; i++) {
            await zoomInBtn.click();
            if (i < 2) {
                await page.waitForTimeout(200);
            }
        }
        await page.mouse.move(0, 0);
        await expect(mainContainer).toHaveScreenshot('zoom-in-unit.png', { maxDiffPixels: 100 });

        await resetBtn.click();
        await page.mouse.move(0, 0);
        await expect(mainContainer).toHaveScreenshot('reset-unit.png', { maxDiffPixels: 100 });
    });

    // 插入 2 个旗子标记，测试标记列表数据
    test('test_markerListData_when_insertTwoFlags', async ({ page, timelinePage }) => {
        const { markerBtn, timelineFrame } = timelinePage;
        const mainContainer = timelineFrame.locator('#main-container');

        await timelineFrame.locator('canvas:nth-child(6)').click({
            position: {
                x: 233,
                y: 4,
            },
        });
        await timelineFrame.locator('canvas:nth-child(6)').click({
            position: {
                x: 449,
                y: 8,
            },
        });

        await markerBtn.click();
        await timelineFrame.getByText('default-0').click();
        await timelineFrame.getByText('default-1').hover();

        await page.mouse.move(0, 0);
        await expect(mainContainer).toHaveScreenshot('marker-list.png', { maxDiffPixels: 100 });

        await page.frameLocator('#Timeline').getByText('Clear').click();
        await page.frameLocator('#Timeline').getByRole('button', { name: 'OK' }).click();

        await page.mouse.move(0, 0);
        await expect(mainContainer).toHaveScreenshot('marker-list-clear.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--Hide/Show All Hidden
    test('test_context_menu_click_hide', async ({ timelinePage }) => {
        const { timelineFrame, clickMenu } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller');
        const clickUnit = unitList.locator('.unit-info').first();
        await clickMenu(clickUnit, timelineFrame, 'Hide');
        const hideUnit = unitList.locator('.unit > .empty');
        expect(await hideUnit.count()).toBe(1);
        await clickMenu(hideUnit, timelineFrame, 'Show All Hidden');
        expect(await hideUnit.count()).toBe(0);
    });

    // 右键菜单--Expand all/Collapse all
    test('test_context_menu_click_ExpandAll', async ({ timelinePage }) => {
        const { timelineFrame, clickMenu } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller');
        const clickUnit = unitList.locator('.unit-info').first();
        await clickMenu(clickUnit, timelineFrame, 'Expand all');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('menu-click-expand-all.png', { maxDiffPixels: 100 });
        await clickMenu(clickUnit, timelineFrame, 'Collapse all');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('menu-click-collapse-all.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--Show in events view
    test('test_context_menu_click_ShowInEventsView', async ({ timelinePage }) => {
        const { timelineFrame, clickMenu, bottomPanel } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller');
        const clickUnit = unitList.locator('.unit-info').nth(1);
        await clickMenu(clickUnit, timelineFrame, 'Show in events view');
        await timelineFrame.locator('.ant-spin').waitFor({ state: 'attached' });
        await timelineFrame.locator('.ant-spin').waitFor({ state: 'detached' });
        await expect(bottomPanel).toHaveScreenshot('test-context-menu-click-ShowInEventsView.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--Undo Zoom/Reset Zoom
    test('test_context_menu_click_zoom', async ({ timelinePage, page }) => {
        const { timelineFrame, clickMenu } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller');
        const clickUnit = unitList.locator('.unit-info').first();
        const options = ['UndoZoom', 'ResetZoom'];

        for (let i = 0; i < options.length; i++) {
            // 聚焦在timeline
            await clickUnit.click({ button: 'middle' });
            await page.keyboard.press('w');
            await page.keyboard.press('w');
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-context-menu-click-Zoom.png', { maxDiffPixels: 100 });
            await clickMenu(clickUnit, timelineFrame, 'Undo Zoom');
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-context-menu-click-${options[i]}.png`, { maxDiffPixels: 100 });
        }
    });

    // 键盘 W、S、A、D、方向键
    test('test_keyword', async ({ timelinePage, page }) => {
        const { timelineFrame, zoomOutBtn } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller');
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();
        await page.keyboard.press('w');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-keyword-w.png`, { maxDiffPixels: 100 });
        await page.keyboard.press('s');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-keyword-s.png`, { maxDiffPixels: 100 });
        await page.keyboard.press('d');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-keyword-d.png`, { maxDiffPixels: 100 });
        await page.keyboard.press('a');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-keyword-a.png`, { maxDiffPixels: 100 });
        await page.keyboard.press('ArrowDown');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-keyword-ArrowDown.png`, { maxDiffPixels: 100 });
        await page.keyboard.press('ArrowUp');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-keyword-ArrowUp.png`, { maxDiffPixels: 100 });
        await zoomOutBtn.click();
        await page.keyboard.press('ArrowRight');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-keyword-ArrowRight.png`, { maxDiffPixels: 100 });
        await page.keyboard.press('ArrowLeft');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-keyword-ArrowLeft.png`, { maxDiffPixels: 100 });
    });

    // Marker小旗子
    test('test_add_marker', async ({ timelinePage, page }) => {
        const { timelineFrame } = timelinePage;
        const flagCanvas = timelineFrame.locator('#timelineFlagCnvas');
        const boundingBox = await flagCanvas.boundingBox();
        if (!boundingBox) {
            return;
        }
        const { x: startX, y: startY } = boundingBox;
        await page.mouse.click(startX + 50, startY + 5);
        await page.mouse.click(startX + 100, startY + 5);
        await page.mouse.click(startX + 200, startY + 5);
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-add-marker.png`, { maxDiffPixels: 100 });
    });

    // 算子调优-框选
    test('test_compute_timeline_selectUnitsRange', async ({ timelinePage, page }) => {
        const { timelineFrame } = timelinePage;
        await importData(page, FilePath.SOURCE);
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();
        const chart = timelineFrame.locator('.chart-selected > div > .canvasContainer > .drawCanvas');
        const boundingBox = await chart.boundingBox();
        if (!boundingBox) {
            return;
        }
        const { x: startX, y: startY } = boundingBox;
        await page.mouse.move(startX + 50, startY + 50);
        await page.mouse.down();
        await page.mouse.move(startX + 200, startX - 200);
        await page.mouse.up();
        await expect(timelineFrame.getByText('Wall Duration', { exact: true })).toBeVisible();
        await expect(timelineFrame.getByText('Self Time')).toBeVisible();
        await expect(timelineFrame.getByText('Average Wall Duration')).toBeVisible();
        const rows = await timelineFrame.locator('.ant-table-row').count();
        expect(rows).toBeGreaterThan(0);
    });

    // 算子调优-点击算子
    test('test_compute_timeline_operator_click', async ({ timelinePage, page }) => {
        const { timelineFrame } = timelinePage;
        await importData(page, FilePath.SOURCE);
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();
        const canvas = timelineFrame.locator('#unitWrapperScroller');
        const boundingBox = await canvas.boundingBox();
        if (!boundingBox) {
            return;
        }
        const { x: startX, y: startY } = boundingBox;
        await page.mouse.click(startX + 332, startY + 350);
        await expect(timelineFrame.getByText('Wall Duration', { exact: true })).toBeVisible();
        await expect(timelineFrame.getByText('Title')).toBeVisible();
        await expect(timelineFrame.getByText('Start')).toBeVisible();
    });
});
