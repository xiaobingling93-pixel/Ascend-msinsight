/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
    test('test_compute_reinforcement_learning_timeline', async ({ page, rLPage }) => {
        const { taskTraceTimelineContent } = rLPage;
        await page.waitForTimeout(100); // 模拟人类操作停顿
        await expect(taskTraceTimelineContent).toHaveScreenshot('select-units-range.png', { maxDiffPixels: 100 });
    });
});
