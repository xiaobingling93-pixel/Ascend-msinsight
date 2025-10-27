/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test as baseTest, expect, WebSocket } from '@playwright/test';
import { TimelinePage, SystemView } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener } from '@/utils';
import { FilePath } from '@/utils/constants';

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

test.describe('Timeline(Joint)', () => {

    test.beforeEach(async ({ page, timelinePage, ws }) => {
        await timelinePage.goto();
        await clearAllData(page);
        await importData(page, FilePath.JOINT_DATA);
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    test('test_timeline_value', async ({ timelinePage, page }) => {
        const { timelineFrame } = timelinePage;
        const firstUnitInfo = timelineFrame.locator('.unit-info').first();
        const secondUnitInfo = timelineFrame.locator('.unit-info').nth(1);
        await page.waitForTimeout(1500);
        await secondUnitInfo.click();
        await firstUnitInfo.hover({ force: true });
        const offsetBtn = timelineFrame.getByTestId('offset-btn').first();
        await offsetBtn.click();
        const offsetInput = timelineFrame.getByRole('tooltip', { name: /Timestamp Offset/i }).getByRole('textbox');
        await offsetInput.fill('300000000');
        await offsetInput.press('Enter');
        await offsetBtn.click();
        await page.mouse.move(0, 0);
    });

    test('test_timeline_systemView', async ({ page, timelinePage }) => {
        const { timelineFrame } = timelinePage;
        const systemViewPage = new SystemView(page);
        await systemViewPage.goto();
        await expect(timelineFrame.getByText('Total Time(us)')).toBeVisible();
        await expect(timelineFrame.getByText('Time Ratio')).toBeVisible();
        await expect(timelineFrame.getByText('Number')).toBeVisible();
        await page.waitForTimeout(9000);
        const rows = await timelineFrame.locator('.ant-table-row').count();
        await expect(rows).toBeGreaterThan(0);
        await page.mouse.move(0, 0);
    });
});
