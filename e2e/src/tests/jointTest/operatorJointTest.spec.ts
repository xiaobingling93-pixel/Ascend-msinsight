/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test as baseTest, expect, WebSocket } from '@playwright/test';
import { OperatorPage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener, waitForWebSocketEvent } from '@/utils';
import { SelectHelpers } from '@/components';
import { FilePath } from '@/utils/constants';

interface TestFixtures {
    operatorPage: OperatorPage;
    ws: Promise<WebSocket>;
}
const test = baseTest.extend<TestFixtures>({
    operatorPage: async ({ page }, use) => {
        const memoryPage = new OperatorPage(page);
        await use(memoryPage);
    },
    ws: async ({ page }, use) => {
        const ws = setupWebSocketListener(page);
        await use(ws);
    },
});

test.describe('Operator(Joint)', () => {

    test.beforeEach(async ({ page, operatorPage, ws }) => {
        const allCardParsedPromise = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        await operatorPage.goto();
        await clearAllData(page);
        await importData(page, FilePath.JOINT_DATA);
        await allCardParsedPromise;
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });
    
    test('filters_Show', async ({ page, operatorPage }) => {
        const { operatorFrame } = operatorPage;
        await expect(operatorFrame.getByText('Group By')).toBeVisible();
        await expect(operatorFrame.getByText('Rank ID')).toBeVisible();
        await expect(operatorFrame.getByText('Top')).toBeVisible();
        await page.mouse.move(0, 0);
    });
    test('change_filterCondition', async ({ page, operatorPage }) => {
        const { operatorFrame, groupIdSelector, rankIdSelector, topSelector } = operatorPage;
        const groupIdSelect = new SelectHelpers(page, groupIdSelector, operatorFrame);
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, operatorFrame);
        const topSelect = new SelectHelpers(page, topSelector, operatorFrame);
        await groupIdSelect.open();
        await groupIdSelect.selectOption('Computing Operator Type');
        await groupIdSelect.open();
        await groupIdSelect.selectOption('Computing Operator');
        await rankIdSelect.open();
        await rankIdSelect.selectOption('0');
        await topSelect.open();
        await topSelect.selectOption('15');
        await topSelect.open();
        await topSelect.selectOption('All');
        await page.mouse.move(0, 0);
    });

    test('expand_table_when_click_seeMoreCell', async ({ page, operatorPage }) => {
        const { operatorFrame, rankIdSelector } = operatorPage;
        await expect(operatorFrame.getByText('Operator Details')).toBeVisible();
        const seeMoreBtn = operatorFrame.getByRole('button', { name: 'See more' }).first();
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, operatorFrame);
        await rankIdSelect.open();
        await rankIdSelect.selectOption('0');
        await seeMoreBtn.click();
        const rows = await operatorFrame.locator('.ant-table-row').count();
        await expect(rows).toBeGreaterThan(0);
        await page.mouse.move(0, 0);
    });

});