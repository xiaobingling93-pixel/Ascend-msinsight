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
import { SummaryPage, FrameworkPage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener, waitForWebSocketEvent } from '@/utils';
import { SelectHelpers } from '@/components';
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
let waitFwdBwdTimelineResponse: Promise<unknown>;

test.describe('Summary(Joint)', () => {
    
    test.beforeEach(async ({ page, summaryPage, ws }) => {
        const { loadingDialog } = new FrameworkPage(page);
        const { fullmask } = summaryPage;
        waitFwdBwdTimelineResponse = waitForWebSocketEvent(page, (res) => res?.type === 'response' && res?.command === 'parallelism/pipeline/fwdBwdTimeline');

        await page.goto('/');
        await importData(page, FilePath.JOINT_DATA);
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

    test('test_baseInfoDisplay', async ({ page, summaryPage }) => {
        const { summaryFrame } = summaryPage;
        const baseInfoContainer = summaryFrame.locator('.mi-collapsible-panel').first();
        await expect(baseInfoContainer.getByText('Report file')).toBeVisible();
        await expect(baseInfoContainer.getByText('Report size(MB)')).toBeVisible();
        await expect(baseInfoContainer.getByText('Profiling session duration')).toBeVisible();
        await page.mouse.move(0, 0);
    });

    test('test_generateRanks', async ({ page, summaryPage }) => {
        const { parallelismGraph } = summaryPage;
        await summaryPage.configureParallel({ algorithm: 'Megatron-LM (tp-cp-ep-dp-pp)', ppSize: 2, tpSize: 2, dpSize: 2, cpSize: 2, epSize: 2 });
        const canvasBox = await parallelismGraph.locator('canvas').first().boundingBox();
        const width = canvasBox?.width ?? 0;
        const height = canvasBox?.height ?? 0;
        expect(width).toBeGreaterThan(0);
        expect(height).toBeGreaterThan(0);
        await page.mouse.move(0, 0);
    });

    test('test_dimensionViews_when_changeDimensionTab', async ({ page, summaryPage }) => {
        const { summaryFrame, parallelismGraph, parallelismGraphLoading } = summaryPage;
        const dimensionTabs = ['DP + PP', 'DP + PP + CP', 'DP + PP + CP + TP'];

        for (const tab of dimensionTabs) {
            await summaryFrame.getByRole('tab', { name: tab, exact: true }).click();
            if (tab !== 'DP') {
                await parallelismGraphLoading.waitFor({ state: 'hidden' });
            }

            await page.mouse.move(0, 0);
            const canvasBox = await parallelismGraph.locator('canvas').first().boundingBox();
            const width = canvasBox?.width ?? 0;
            const height = canvasBox?.height ?? 0;
            expect(width).toBeGreaterThan(0);
            expect(height).toBeGreaterThan(0);
        }
        await page.mouse.move(0, 0);
    });

    test('test_heatmap_given_preparingIndicator', async ({ page, summaryPage }) => {
        const { summaryFrame } = summaryPage;
        const dataTypeSelect = summaryFrame.locator('#dataType');
        const dataTypeSelector = new SelectHelpers(page, dataTypeSelect, summaryFrame);
        await summaryPage.changeDimensionTo('tp');
        await dataTypeSelector.open();
        await dataTypeSelector.selectOption('Preparing');
        const dyeingMinimumInput = summaryFrame.getByTestId('input-dyeing-minimum');
        const dyeingMaximumInput = summaryFrame.getByTestId('input-dyeing-maximum');
        await dyeingMinimumInput.fill('700');
        await dyeingMaximumInput.fill('900');
        await page.mouse.move(0, 0);
    });

    test('test_performanceChartDisplay', async ({ page, summaryPage }) => {
        const { performanceChart } = summaryPage;
        const canvasBox = await performanceChart.locator('canvas').boundingBox();
        const width = canvasBox.width;
        const height = canvasBox.height;
        expect(width).toBeGreaterThan(0);
        expect(height).toBeGreaterThan(0);
        await page.mouse.move(0, 0);
    });

});