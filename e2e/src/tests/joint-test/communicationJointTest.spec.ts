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
import { CommunicationPage, FrameworkPage, TimelinePage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener, waitForResponse, waitForWebSocketEvent } from '@/utils';
import { SelectHelpers, TableHelpers } from '@/components';
import { FilePath } from '@/utils/constants';

interface TestFixtures {
    communicationPage: CommunicationPage;
    ws: Promise<WebSocket>;
}
const test = baseTest.extend<TestFixtures>({
    communicationPage: async ({ page }, use) => {
        const communicationPage = new CommunicationPage(page);
        await use(communicationPage);
    },
    ws: async ({ page }, use) => {
        const ws = setupWebSocketListener(page);
        await use(ws);
    },
});
let requestDurationListResp: Promise<unknown>;
let requestTableDataResp: Promise<unknown>;
let allPagesSuccessRes: Promise<unknown>;

test.describe('Communication(Joint)', () => {

    test.beforeEach(async ({ page, communicationPage, ws }) => {
        requestDurationListResp = waitForWebSocketEvent(page, (res) => res?.command === 'communication/duration/list');
        requestTableDataResp = waitForWebSocketEvent(page, (res) => res?.command === 'communication/operatorDetails');
        allPagesSuccessRes = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');

        const { loadingDialog } = new FrameworkPage(page);
        await page.goto('/');
        await importData(page, FilePath.JOINT_DATA);
        await communicationPage.goto();
        await page.mouse.move(0, 0);
        if (await loadingDialog.count()) {
            await loadingDialog.waitFor({ state: 'detached' });
        }
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    test('configuration_show', async ({ page, communicationPage }) => {
        test.setTimeout(90_000);
        const {
            communicationFrame,
            communicationMatrixRadio,
            durationAnalysisRadio,
        } = communicationPage;
        await expect(communicationFrame.getByText('Cluster')).toBeVisible();
        await expect(communicationFrame.getByText('Step')).toBeVisible();
        await expect(communicationFrame.getByText('Communication Group')).toBeVisible();
        await expect(communicationFrame.getByText('Operator Name')).toBeVisible();
        await expect(communicationMatrixRadio).toBeVisible();
        await expect(durationAnalysisRadio).toBeVisible();
    });

    test('matrix_model_show', async ({ page, communicationPage }) => {
        const {
            communicationFrame,
        } = communicationPage;
        await expect(communicationFrame.getByText('Communication Matrix Type')).toBeVisible();
        await expect(communicationFrame.getByText('Show Inner Communication')).toBeVisible();
        await expect(communicationFrame.getByText('Visible Range')).toBeVisible();
    });

    test('duration_analysis_show', async ({ page, communicationPage }) => {
        const { communicationMatrixRadio, durationAnalysisRadio, communicationFrame, switchDurationAnalysis } = communicationPage;
        await switchDurationAnalysis(communicationMatrixRadio, durationAnalysisRadio);
        await expect(communicationFrame.getByText('Slow Rank List')).toBeVisible();
        await expect(communicationFrame.getByText('Visualized Communication Time')).toBeVisible();
        await expect(communicationFrame.getByText('Data Analysis of Communication Time')).toBeVisible();

    });

});
