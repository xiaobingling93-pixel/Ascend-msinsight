/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test as baseTest, expect, WebSocket } from '@playwright/test';
import { TimelinePage, SystemView, CommunicationPage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener, waitForWebSocketEvent } from '@/utils';
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
let allPagesSuccessRes: Promise<unknown>;
test.describe('Timeline', () => {
    test.beforeEach(async ({ page, timelinePage, ws }) => {
        allPagesSuccessRes = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        const { timelineFrame } = timelinePage;
        await timelinePage.goto();
        await clearAllData(page);
        await importData(page);
        const secondLayerUnit = timelineFrame.locator('#main-container').getByText('Python (2045554)');
        await expect(secondLayerUnit).toBeVisible();
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    // 树状图 - 泳道展开收缩
    test('test_unitsExpandAndCollapse_when_click', async ({ timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const secondLayerUnit = timelineFrame.locator('#main-container').getByText('Python (2045554)');
        const firstUnitInfo = timelineFrame.locator('.unit-info').first();
        await firstUnitInfo.click();
        await expect(secondLayerUnit).toBeHidden();
        await firstUnitInfo.click();
        await expect(secondLayerUnit).toBeVisible();
    });

    // 树状图 - 泳道置顶
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

    // 树状图 - 偏移量设置
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

    // System View - 点击算子展示算子详情
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

    // System View - 框选泳道展示算子列表
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
            'Overall Metrics',
            'Python API Summary',
            'CANN API Summary',
            'Ascend HardWare Task Summary',
            'Communication Summary',
            'Overlap Analysis',
            'Kernel Details',
        ];

        await expect(bottomPanel).toHaveScreenshot('StatsSystemView-Overall-Metrics.png', { maxDiffPixels: 400 });

        for (const item of statsSystemViewOptions) {
            const option = timelineFrame.getByText(item, { exact: true });
            await option.click();
            await timelineFrame.locator('.ant-spin').waitFor({ state: 'hidden' });
            await expect(bottomPanel).toHaveScreenshot(`StatsSystemView-${item}.png`, { maxDiffPixels: 400 });
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

        const expertSystemViewOptions = ['Expert Analysis', 'Affinity API', 'Affinity Optimizer',
            'AICPU Operators', 'ACLNN Operators', 'Operators Fusion', 'Operators Dispatch'];

        for (const item of expertSystemViewOptions) {
            const option = timelineFrame.getByText(item, { exact: true });
            await option.click();
            await timelineFrame.locator('.ant-spin').waitFor({ state: 'hidden' });
            await expect(bottomPanel).toHaveScreenshot(`ExpertSystemView-${item}.png`, { maxDiffPixels: 400 });
        }
    });

    // 工具栏 - 算子搜索
    test('test_operatorSearch_when_EnterOperatorName', async ({ page, timelinePage }) => {
        await allPagesSuccessRes;
        const { searchBtn, timelineFrame } = timelinePage;
        await searchBtn.click();
        const inputLocator = timelineFrame.locator('.insight-category-search-overlay input');
        const input = new InputHelpers(page, inputLocator, timelineFrame);
        await input.setValue('add');
        await input.press('Enter');
        await page.mouse.move(0, 0);
        await page.waitForTimeout(2000);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('search-operator.png', { maxDiffPixels: 50 });
    });

    // 工具栏 - 算子搜索在泳道较深的位置时能显示在div中
    test('test_deepOperatorSearch_when_EnterOperatorName', async ({ page, timelinePage }) => {
        await allPagesSuccessRes;
        const { searchBtn, timelineFrame } = timelinePage;
        await searchBtn.click();
        const inputLocator = timelineFrame.locator('.insight-category-search-overlay input');
        const input = new InputHelpers(page, inputLocator, timelineFrame);
        await input.setValue('<built-in function get_autocast_gpu_dtype>');
        await input.press('Enter');
        await page.mouse.move(0, 0);
        await page.waitForTimeout(2000);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('search-deep-operator.png', { maxDiffPixels: 50 });
    });

    // 工具栏 - 算子连线  HostToDevice MsTx async_npu
    test('test_operator_npu_LinkLine', async ({ page, timelinePage }) => {
        const { flowBtn, timelineFrame } = timelinePage;
        const hcclUnit = timelineFrame.locator('#unitWrapperScroller').getByText('Ascend Hardware (2094647552)');
        await hcclUnit.click();
        const LinkLineType = [
            'HostToDevice',
            'MsTx',
            'async_npu',
        ];
        for (const item of LinkLineType) {
            await flowBtn.click();
            const LinkLineTypeCheckbox = timelineFrame.getByLabel(item);
            await LinkLineTypeCheckbox.check();
            await flowBtn.click();
            await page.mouse.move(0, 0);
            await page.waitForTimeout(2000);
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`operator-link-line-${item}.png`, { maxDiffPixels: 100 });
            await flowBtn.click();
            await LinkLineTypeCheckbox.uncheck();
            await flowBtn.click();
        }
    });

    // 工具栏 - 算子连线  async_task_queue fwdbwd
    test('test_operator_cpu_LinkLine', async ({ page, timelinePage }) => {
        const { flowBtn, timelineFrame } = timelinePage;
        const hcclUnit = timelineFrame.locator('#unitWrapperScroller').getByText('Python (2045554)');
        await hcclUnit.click();
        const LinkLineType = [
            'async_task_queue',
            'fwdbwd',
        ];
        for (const item of LinkLineType) {
            await flowBtn.click();
            const LinkLineTypeCheckbox = timelineFrame.getByLabel(item);
            await LinkLineTypeCheckbox.check();
            await flowBtn.click();
            await page.mouse.move(0, 0);
            await page.waitForTimeout(2000);
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`operator-link-line-${item}.png`, { maxDiffPixels: 100 });
            await flowBtn.click();
            await LinkLineTypeCheckbox.uncheck();
            await flowBtn.click();
        }
    });

    // 工具栏 - 泳道(card)过滤
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

    // 工具栏 - 泳道(unit)过滤
    test('test_unitFilter', async ({ page, timelinePage }) => {
        await allPagesSuccessRes;
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

    // 工具栏 - 缩放按钮、重置按钮
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

    // 工具栏 - 插入 2 个旗子标记，测试标记列表数据
    test('test_markerListData_when_insertTwoFlags', async ({ page, timelinePage }) => {
        const { markerBtn, timelineFrame } = timelinePage;
        const mainContainer = timelineFrame.locator('#main-container');

        await timelineFrame.locator('canvas:nth-child(4)').click({
            position: {
                x: 233,
                y: 4,
            },
        });
        await timelineFrame.locator('canvas:nth-child(4)').click({
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

        await timelineFrame.getByText('Clear').click();
        await timelineFrame.getByRole('button', { name: 'OK' }).click();

        await page.mouse.move(0, 0);
        await expect(mainContainer).toHaveScreenshot('marker-list-clear.png', { maxDiffPixels: 100 });
    });

    // 工具栏 - Marker小旗子
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
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-add-marker.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--Fit to screen
    test('test_fitToScreen_when_rightClickOperator', async ({ timelinePage, page }) => {
        const { timelineFrame, mainContainer, unitWrapperScroller } = timelinePage;
        const secondLayerUnit = mainContainer.getByText('Python (2045554)');

        await secondLayerUnit.click();
        await unitWrapperScroller
            .locator('canvas')
            .nth(1)
            .click({
                button: 'right',
                position: {
                    x: 251,
                    y: 89,
                },
            });
        await timelineFrame.getByText('Fit to screen').click();
        await page.mouse.move(0, 0);

        await expect(mainContainer).toHaveScreenshot('fit-to-screen.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--右键点击通信算子跳转至通信页面
    test('test_redirectToCommunication_when_rightClickHCCLOperator', async ({ page, timelinePage }) => {
        const { timelineFrame, unitWrapperScroller } = timelinePage;
        const { communicationFrame } = new CommunicationPage(page);

        await unitWrapperScroller.getByText('HCCL (2094647744)').click();
        await timelineFrame.locator('div:nth-child(12) > .chart > div > .canvasContainer > .drawCanvas').click({
            button: 'right',
            position: {
                x: 75,
                y: 9,
            },
        });
        await timelineFrame.getByText('Find in Communication').click();
        await page.waitForTimeout(1000);

        const hcclChart = communicationFrame.locator('.panel-content').first();
        await expect(hcclChart).toHaveScreenshot('redirect-to-communication.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--Hide/Show all
    test('test_context_menu_click_hide', async ({ timelinePage }) => {
        const { timelineFrame, clickMenu } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller');
        const clickUnit = unitList.locator('.unit-info').first();
        await clickMenu(clickUnit, timelineFrame, 'Hide');
        const hideUnit = unitList.locator('.unit > .empty');
        expect(await hideUnit.count()).toBe(1);
        await clickMenu(hideUnit, timelineFrame, 'Show all');
        expect(await hideUnit.count()).toBe(0);
    });

    // 右键菜单--多选隐藏有兄弟关系 Hide/Show all
    test('test_context_menu_click_multi_hide_siblings', async ({ timelinePage }) => {
        const { timelineFrame, clickMenu } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller .unit-info');
        const clickUnitOne = unitList.nth(1);
        const clickUnitTwo = unitList.nth(2);

        await clickUnitOne.hover();
        const clickUnitCheckboxOne = clickUnitOne.locator('input[type="checkbox"]');
        await clickUnitCheckboxOne.click({ button: 'left' });
        expect(await clickUnitCheckboxOne.isChecked()).toBe(true);
        await clickUnitTwo.hover();
        const clickUnitCheckboxTwo = clickUnitTwo.locator('input[type="checkbox"]');
        await clickUnitCheckboxTwo.click({ button: 'left' });
        expect(await clickUnitCheckboxTwo.isChecked()).toBe(true);
        await clickMenu(clickUnitTwo, timelineFrame, 'Hide');
        const hideUnit = timelineFrame.locator('#unitWrapperScroller .unit > .empty');
        const hideUnitTitle = hideUnit.locator('.insight-lane-info span');
        expect(await hideUnitTitle.innerHTML()).toBe('2 units hidden');
        await clickMenu(hideUnit, timelineFrame, 'Show all');
        expect(await hideUnit.count()).toBe(0);
    });

    // 右键菜单--多选隐藏有父子关系 Hide/Show all
    test('test_context_menu_click_multi_hide_parent_child', async ({ timelinePage }) => {
        const { timelineFrame, clickMenu } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller .unit-info');
        const clickUnitParent = unitList.nth(0);
        const clickUnitChild = unitList.nth(1);

        await clickUnitParent.hover();
        const clickUnitCheckboxParent = clickUnitParent.locator('input[type="checkbox"]');
        await clickUnitCheckboxParent.click({ button: 'left' });
        expect(await clickUnitCheckboxParent.isChecked()).toBe(true);
        await clickUnitChild.hover();
        const clickUnitCheckboxChild = clickUnitChild.locator('input[type="checkbox"]');
        await clickUnitCheckboxChild.click({ button: 'left' });
        expect(await clickUnitCheckboxChild.isChecked()).toBe(true);
        await clickMenu(clickUnitChild, timelineFrame, 'Hide');

        const hideUnit = timelineFrame.locator('#unitWrapperScroller .unit > .empty');
        expect(await hideUnit.count()).toBe(1);
        await clickMenu(hideUnit, timelineFrame, 'Show all');
        expect(await hideUnit.count()).toBe(0);
    });

    // 右键菜单--Expand all/Collapse all
    test('test_context_menu_click_ExpandAll', async ({ timelinePage, page }) => {
        const { timelineFrame, clickMenu } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller');
        const clickUnit = unitList.locator('.unit-info').first();
        await clickMenu(clickUnit, timelineFrame, 'Expand all');
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('menu-click-expand-all.png', { maxDiffPixels: 100 });
        await clickMenu(clickUnit, timelineFrame, 'Collapse all');
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('menu-click-collapse-all.png', { maxDiffPixels: 100 });
    });

    // 右键菜单--Show in events view
    test('test_context_menu_click_ShowInEventsView', async ({ timelinePage, page }) => {
        const { timelineFrame, clickMenu, bottomPanel } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller');
        const clickUnit = unitList.locator('.unit-info').nth(1);
        await clickMenu(clickUnit, timelineFrame, 'Show in Events View');
        await timelineFrame.locator('.ant-spin').waitFor({ state: 'attached' });
        await timelineFrame.locator('.ant-spin').waitFor({ state: 'detached' });
        await page.mouse.move(0, 0);
        await expect(bottomPanel).toHaveScreenshot('test-context-menu-click-ShowInEventsView.png', { maxDiffPixels: 400 });
    });

    // 右键菜单--Undo Zoom/Reset Zoom
    test('test_context_menu_click_zoom', async ({ timelinePage, page }) => {
        const { timelineFrame, clickMenu } = timelinePage;
        const unitList = timelineFrame.locator('#unitWrapperScroller');
        const clickUnit = unitList.locator('.unit-info').first();
        const options = ['Undo Zoom', 'Reset Zoom'];

        for (let i = 0; i < options.length; i++) {
            // 聚焦在timeline
            await clickUnit.click({ button: 'left' }); // 选中unit+收起unit
            await clickUnit.click({ button: 'left' }); // 选中unit+展开unit
            await page.keyboard.press('w');
            await page.waitForTimeout(1000);
            await page.mouse.move(0, 0);
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-context-menu-click-Zoom.png', { maxDiffPixels: 200 });
            await clickMenu(clickUnit, timelineFrame, options[i]);
            await page.mouse.move(0, 0);
            await page.waitForTimeout(1000);
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot(`test-context-menu-click-${options[i]}.png`, { maxDiffPixels: 200 });
        }
    });

    // 快捷键 - 键盘 W、S、A、D、方向键
    test('test_keyword', async ({ timelinePage, page }) => {
        const { timelineFrame, zoomOutBtn, resetBtn } = timelinePage;
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();
        await page.mouse.move(0, 0);
        await page.keyboard.press('w');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-w.png', { maxDiffPixels: 100 });
        await page.keyboard.press('d');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-d.png', { maxDiffPixels: 100 });
        await page.keyboard.press('a');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-a.png', { maxDiffPixels: 100 });
        await page.keyboard.press('s');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-s.png', { maxDiffPixels: 100 });
        await page.keyboard.press('ArrowDown');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-ArrowDown.png', { maxDiffPixels: 100 });
        await page.keyboard.press('ArrowUp');
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-ArrowUp.png', { maxDiffPixels: 100 });
        await resetBtn.click();
        await zoomOutBtn.click();
        await page.mouse.move(0, 0);
        await page.keyboard.press('ArrowRight');
        try {
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-ArrowRight.png', { maxDiffPixels: 100 });
        } catch (e) {
            await page.keyboard.press('ArrowRight');
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-ArrowRight.png', { maxDiffPixels: 100 });
        }
        await page.keyboard.press('ArrowLeft');
        try {
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-ArrowLeft.png', { maxDiffPixels: 100 });
        } catch (e) {
            await page.keyboard.press('ArrowLeft');
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-keyword-ArrowLeft.png', { maxDiffPixels: 100 });
        }
    });

    // 快捷键 - 测试快捷键 Q
    test('test_q', async ({ timelinePage, page }) => {
        const { bottomPanel, timelineFrame } = timelinePage;
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await page.mouse.move(0, 0);
        await secondUnitInfo.click();
        await page.keyboard.press('q');
        await expect(bottomPanel).toHaveScreenshot('test-keyword-q.png', { maxDiffPixels:100 });
    });

    // 图形化窗格 - 测试直方图的显示 如NPU_MEM
    test('test_npu_mem', async ({ timelinePage, page }) => {
        const { timelineFrame } = timelinePage;
        const npuMemLayerUnit = timelineFrame.locator('#main-container').getByText('NPU_MEM (2094647712)');
        await npuMemLayerUnit.click();
        await page.mouse.wheel(0, 250);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test-npu-mem.png', { maxDiffPixels:100 });
    });

    // 图形化窗格 - 锁定选中区间后，点击算子跳转显示选中详情
    test('test_lock_selected_area_then_click_operator_detail', async ({ timelinePage, page }) => {
        const { timelineFrame, clickMenu } = timelinePage;
        const clickUnit = timelineFrame.locator('.unit-info').nth(9);
        await clickUnit.click();
        // 这里需要优化，改为当图表渲染完成后再继续执行
        await page.waitForTimeout(1000);
        const chart = timelineFrame.locator('.chart-selected > div > .canvasContainer > .drawCanvas');
        const chartInfo = await chart.boundingBox();
        if (!chartInfo) {
            return;
        }
        const { x: startX, y: startY } = chartInfo;
        await page.mouse.move(startX + 50, startY + 80);
        await page.mouse.down();
        await page.mouse.move(startX + 200, startY + 150);
        await page.mouse.up();
        // 右键锁定
        await clickMenu(chart.first(), timelineFrame, 'Lock selection area');
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('test_lock_selected_area.png', { maxDiffPixels:100 });
        // 点击算子
        await chart.first().click({
            position: {
                x: 316,
                y: 12,
            },
        });
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('.bottomPanelContainer ')).toHaveScreenshot('test_lock_selected_area_then_click_operator_detail.png', { maxDiffPixels:100 });
    });
});

