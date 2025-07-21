/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test as baseTest, expect, WebSocket } from '@playwright/test';
import { LeaksPage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener } from '@/utils';
import { FilePath } from '@/utils/constants';
import { SelectHelpers } from '@/components';

interface TestFixtures {
    leaksPage: LeaksPage;
    ws: Promise<WebSocket>;
}

const test = baseTest.extend<TestFixtures>({
    leaksPage: async ({ page }, use) => {
        const leaksPage = new LeaksPage(page);
        await use(leaksPage);
    },
    ws: async ({ page }, use) => {
        const ws = setupWebSocketListener(page);
        await use(ws);
    },
});

test.describe('Leaks_Test', () => {
    test.beforeEach(async ({ page, leaksPage, ws }) => {
        await page.goto('/');
        await importData(page, FilePath.LEAKS_DUMP);
        await leaksPage.goto();
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    // 切换线程
    test('test_change_ThreadID', async ({ leaksPage, page }) => {
        const { leaksFrame, threadIdSelector } = leaksPage;
        const threadIdSelect = new SelectHelpers(page, threadIdSelector, leaksFrame);
        await threadIdSelect.open();
        await threadIdSelect.selectOption('3842521');
        await page.mouse.move(0, 0);
        await expect(leaksFrame.locator('#funcContent')).toHaveScreenshot('funcContent.png', { maxDiffPixels: 100 });
    });

    // 切换线程
    test('test_change_DeviceID', async ({ leaksPage, page }) => {
        const { leaksFrame, deviceIdSelector } = leaksPage;
        const deviceIdSelect = new SelectHelpers(page, deviceIdSelector, leaksFrame);
        await deviceIdSelect.open();
        await deviceIdSelect.selectOption('1');
        await page.mouse.move(0, 0);
        await expect(leaksFrame.locator('#barContent')).toHaveScreenshot('barContent_deviceId.png', { maxDiffPixels: 100 });
    });

    // 切换类型
    test('test_change_type', async ({ leaksPage, page }) => {
        const { leaksFrame, typeSelector } = leaksPage;
        const typeSelect = new SelectHelpers(page, typeSelector, leaksFrame);
        await typeSelect.open();
        await typeSelect.selectOption('HAL');
        await page.mouse.move(0, 0);
        await expect(leaksFrame.locator('#barContent')).toHaveScreenshot('barContent_type.png', { maxDiffPixels: 100 });
    });

    // 内存拆解图展示
    test('test_memorySlice_show', async ({ leaksPage, page }) => {
        const { leaksFrame } = leaksPage;
        // 等待 echarts 动画结束
        await page.waitForTimeout(1000);
        await leaksFrame.locator('#barContent canvas').click({ position: { x: 526, y: 217 } });
        await expect(leaksFrame.locator('#detailsContent')).toHaveScreenshot('detailsContent.png', { maxDiffPixels: 100 });
    });
});
