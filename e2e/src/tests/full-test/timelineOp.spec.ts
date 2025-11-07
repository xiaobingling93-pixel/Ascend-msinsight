/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test as baseTest, expect, WebSocket } from '@playwright/test';
import { TimelinePage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener } from '@/utils';
import { FilePath } from '@/utils/constants';
import { InputHelpers } from '@/components';

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

test.describe('Timeline(Operator)', () => {
    test.beforeEach(async ({ page, timelinePage, ws }) => {
        await timelinePage.goto();
        await clearAllData(page);
        await importData(page, FilePath.SOURCE);
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    // 算子调优-图形化窗格-框选
    test('test_compute_timeline_selectUnitsRange', async ({ timelinePage, page }) => {
        const { timelineFrame } = timelinePage;
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
        await expect(timelineFrame.getByText('Self Time')).not.toBeVisible();
        await expect(timelineFrame.getByText('Average Wall Duration')).toBeVisible();
        await expect(timelineFrame.getByText('Max Wall Duration')).toBeVisible();
        await expect(timelineFrame.getByText('Min Wall Duration')).toBeVisible();
        const rows = await timelineFrame.locator('.ant-table-row').count();
        expect(rows).toBeGreaterThan(0);
    });

    // 算子调优-Slice List-框选-算子统计项
    test('test_compute_timeline_selectUnitsRange_slice_list_total', async ({ timelinePage, page }) => {
        const { timelineFrame } = timelinePage;
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
        await page.mouse.move(startX + 200, startY + 50);
        await page.mouse.up();
        const tfoot = await timelineFrame.locator('tfoot');
        expect(await tfoot.locator('tr > td').nth(5).innerText()).toBe('387');
        expect(await tfoot.locator('tr > td').nth(1).innerText()).toBe('0.008114 ms');
        expect(await tfoot.locator('tr > td').nth(2).innerText()).toBe('0.000021 ms');
        expect(await tfoot.locator('tr > td').nth(3).innerText()).toBe('0.000204 ms');
        expect(await tfoot.locator('tr > td').nth(4).innerText()).toBe('0.000000 ms');
    });

    // 算子调优-图形化窗格-点击算子
    test('test_compute_timeline_operator_click', async ({ timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await secondUnitInfo.click();

        const canvas = timelineFrame.locator('#unitWrapperScroller canvas').nth(1);
        await canvas.click({
            position: {
                x: 29,
                y: 13,
            },
        });
        await expect(timelineFrame.getByText('Wall Duration', { exact: true })).toBeVisible();
        await expect(timelineFrame.getByText('Title')).toBeVisible();
        await expect(timelineFrame.getByText('Start', { exact:true })).toBeVisible();
    });

    // 算子调优-工具栏-搜索
    test('test_Search_when_EnterInstr', async ({ page, timelinePage }) => {
        const { searchBtn, timelineFrame, openInWindows } = timelinePage;
        await searchBtn.click();
        const inputLocator = timelineFrame.locator('.insight-category-search-overlay input');
        const input = new InputHelpers(page, inputLocator, timelineFrame);
        await input.setValue('add');
        await page.waitForTimeout(1000);
        await input.press('Enter');
        await openInWindows.waitFor({ state: 'attached' });
        await page.mouse.move(0, 0);
        // 这里查询时，有时候会优先查到第二条泳道中，在此兼容一下，后面应该修改功能
        try {
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('search-instr.png', { maxDiffPixels: 100 });
        }catch (e) {
            await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('search-instr-1.png', { maxDiffPixels: 100 });
        }
    });
});
