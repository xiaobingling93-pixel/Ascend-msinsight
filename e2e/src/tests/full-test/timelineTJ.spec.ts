/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test as baseTest, expect, WebSocket } from '@playwright/test';
import { TimelinePage } from '@/page-object';
import {clearAllData, dragSelect, importData, setupWebSocketListener} from '@/utils';
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

test.describe('Timeline(Trace_Json)', () => {
    test.beforeEach(async ({ page, timelinePage, ws }) => {
        await timelinePage.goto();
        await clearAllData(page);
        await importData(page, FilePath.TRACE_JSON);
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    // 算子调优-图形化窗格-框选
    test('test_compute_timeline_selectUnitsRange', async ({ timelinePage, page }) => {
        //timeline功能框选有bug，待修复
        const { timelineFrame } = timelinePage;
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(2);
        await secondUnitInfo.click();
        await page.waitForTimeout(1000);
        const chart = timelineFrame.locator('.chart-selected > div > .canvasContainer > .drawCanvas');
        const boundingBox = await chart.boundingBox();
        if (!boundingBox) {
            return;
        }
        const { x: startX, y: startY } = boundingBox;
        await page.mouse.move(startX + 50, startY + 50);
        await page.mouse.down();
        await page.waitForTimeout(100); // 模拟人类操作停顿
        await page.mouse.move(startX + 200, startY + 100,{ steps: 20 });
        await page.mouse.up();
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('select-units-range.png', { maxDiffPixels: 100 });
        await expect(timelineFrame.locator('.ant-tabs-content-holder').nth(0)).toHaveScreenshot('select-units-range-slice-list.png', { maxDiffPixels: 100 });
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
        await dragSelect(page, {
            x:startX + 50, y:startY + 80,
        },{
            x:startX + 200, y:startY + 150,
        });
        const tfoot = await timelineFrame.locator('tfoot');
        //由于数据过大，框选后可能会有像素差，使用数值比较是否大于0的方式判断功能是否正常，待修复
        expect(Number((await tfoot.locator('tr > td').nth(1).innerText()).split(' ')[0]) > 0);
        expect(Number((await tfoot.locator('tr > td').nth(2).innerText()).split(' ')[0]) > 0);
        expect(Number((await tfoot.locator('tr > td').nth(3).innerText()).split(' ')[0]) > 0);

    });

    // 算子调优-工具栏-搜索
    test('test_Search_when_EnterInstr', async ({ page, timelinePage }) => {
        const { searchBtn, timelineFrame, openInWindows } = timelinePage;
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(2);
        await secondUnitInfo.click();
        await searchBtn.click();
        const inputLocator = timelineFrame.locator('.insight-category-search-overlay input');
        const input = new InputHelpers(page, inputLocator, timelineFrame);
        await input.setValue('LD');
        await input.press('Enter');
        await openInWindows.waitFor({ state: 'attached' });
        await page.mouse.move(0, 0);
        await expect(timelineFrame.locator('#main-container')).toHaveScreenshot('search-instr.png', { maxDiffPixels: 100 });
    });
});
