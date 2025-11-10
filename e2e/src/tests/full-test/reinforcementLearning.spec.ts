/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test as baseTest, expect, WebSocket } from '@playwright/test';
import { RLPage } from '@/page-object';
import {clearAllData, importData, setupWebSocketListener} from '@/utils';
import { FilePath } from '@/utils/constants';

interface TestFixtures {
    rLPage: RLPage;
    ws: Promise<WebSocket>;
}

const test = baseTest.extend<TestFixtures>({
    rLPage: async ({ page }, use) => {
        const rLPage = new RLPage(page);
        await use(rLPage);
    },
    ws: async ({ page }, use) => {
        const ws = setupWebSocketListener(page);
        await use(ws);
    },
});

test.describe('Reinforcement-learning', () => {
    test.beforeEach(async ({ page, rLPage, ws }) => {
        await rLPage.goto('timeline');
        await clearAllData(page);
        await importData(page, FilePath.REINFORCEMENT_LEARNING);
        await page.waitForTimeout(5000); // 模拟人类操作停顿
        await rLPage.goto('RL');
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    // 算子调优-图形化窗格-框选
    test('test_compute_reinforcement_learning_timeline', async ({ rLPage, page }) => {
        const { rLFrame } = rLPage;
        await page.waitForTimeout(100); // 模拟人类操作停顿
        await expect(rLFrame.locator('#task-trace-timeline')).toHaveScreenshot('select-units-range.png', { maxDiffPixels: 100 });
    });
});
