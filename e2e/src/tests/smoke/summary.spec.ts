/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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
import { SummaryPage, FrameworkPage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener, waitForWebSocketEvent } from '@/utils';
import { FilePath } from '@/utils/constants';

interface TestFixtures {
    summaryPage: SummaryPage;
    ws: Promise<WebSocket>;
}
const test = baseTest.extend<TestFixtures>({
    summaryPage: async ({ page }, use) => {
        const summaryPage = new SummaryPage(page);
        await use(summaryPage);
    },
    ws: async ({ page }, use) => {
        const ws = setupWebSocketListener(page);
        await use(ws);
    },
});
let allPagesSuccessRes: Promise<unknown>;
let clusterCompletedRes: Promise<unknown>;
test.describe('Summary', () => {
    test.beforeEach(async ({ page, summaryPage, ws }) => {
        allPagesSuccessRes = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        clusterCompletedRes = waitForWebSocketEvent(page, (res) => res?.event === 'parse/clusterCompleted');
        
        const { loadingDialog } = new FrameworkPage(page);
        const { fullmask } = summaryPage;

        await page.goto('/');
        await importData(page, FilePath.SMOKE_DATA);
        await allPagesSuccessRes;
        await clusterCompletedRes;
        await summaryPage.goto();
        if (await loadingDialog.count()) {
            await loadingDialog.waitFor({ state: 'attached' });
        }
        if (await fullmask.count()) {
            await fullmask.waitFor({ state: 'hidden' });
        }
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    // 展示并行策略图和计算通信概览柱状图
    test('display parallelism and performance chart', async ({ page, summaryPage }) => {
        await expect(page.locator('iframe[name="Summary"]').contentFrame().getByTestId('parallelism-graph-placeholder')).toBeVisible();
        await expect(page.locator('iframe[name="Summary"]').contentFrame().getByTestId('performance-chart').locator('canvas')).toBeVisible();
    });
});