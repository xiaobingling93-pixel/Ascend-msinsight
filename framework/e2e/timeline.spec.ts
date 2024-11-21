/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { test as baseTest, expect } from '@playwright/test';
import { TimelinePage, SystemView } from './page-object';
import { clearAllData, importData, waitForWebSocketEvent } from './utils';

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

    // 泳道展开收缩
    test('units expand and collapse', async ({ timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const secondLayerUnit = timelineFrame.locator('#main-container').getByText('Python (2045554)');
        const firstUnitInfo = timelineFrame.locator('.unit-info').first();
        await firstUnitInfo.click();
        await expect(secondLayerUnit).toBeHidden();
        await firstUnitInfo.click();
        await expect(secondLayerUnit).toBeVisible();
    });

    // 泳道置顶
    test('units pin to top and unpin', async ({ timelinePage }) => {
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
    test('offset config', async ({ page, timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const firstUnitInfo = timelineFrame.locator('.unit-info').first();
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();
        await firstUnitInfo.hover({ force: true });
        // 这里需要优化，改为当图表渲染完成后再继续执行
        await page.waitForTimeout(1500);
        const offsetBtn = timelineFrame.getByTestId('offset-btn');
        await offsetBtn.click();
        const offsetInput = timelineFrame.getByRole('tooltip', { name: /Timestamp Offset/i }).getByRole('textbox');
        await offsetInput.fill('300000000');
        await offsetInput.press('Enter');
        await offsetBtn.click();
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('unit-offset.png');
    });

    // 点击算子展示算子详情
    test('click operator to show Slice Detail ', async ({ page, timelinePage }) => {
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
    test('select units range to show Slice List', async ({ page, timelinePage }) => {
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
    // System View数据展示
    test('System View data display', async ({ page }) => {
        const systemView = new SystemView(page);
        await systemView.goto();
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });
});
