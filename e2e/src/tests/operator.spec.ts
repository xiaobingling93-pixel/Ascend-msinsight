/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test as baseTest, expect, WebSocket } from '@playwright/test';
import { OperatorPage } from '@/page-object';
import { clearAllData, importData, setCompare, setupWebSocketListener, waitForWebSocketEvent } from '@/utils';
import { SelectHelpers } from '@/components';

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

const operatorImgMap = {
    loadOperatorDataSuccess: 'operator.png',
    expandOperatorDetailTableDataSuccess: 'operator-expand-detail.png',
    compareRankRes: 'operator-compare-rank.png',
};

test.describe('Operator', () => {
    test.beforeEach(async ({ page, operatorPage, ws }) => {
        const allCardParsedPromise = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        await operatorPage.goto();
        await clearAllData(page);
        await importData(page);
        await allCardParsedPromise;
    });

    // 【case】非对比非多机operator界面加载
    test('change_filterCondition', async ({ page, operatorPage }) => {
        const { operatorFrame, groupIdSelector, rankIdSelector, topSelector } = operatorPage;
        const groupIdSelect = new SelectHelpers(page, groupIdSelector, operatorFrame);
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, operatorFrame);
        const topSelect = new SelectHelpers(page, topSelector, operatorFrame);
        await groupIdSelect.open();
        await groupIdSelect.selectOption('Computing Operator Type');
        await rankIdSelect.open();
        await rankIdSelect.selectOption('4 4');
        await topSelect.open();
        await topSelect.selectOption('15');
        await page.mouse.move(0, 0);
        await expect(operatorFrame.locator('.mi-page')).toHaveScreenshot(operatorImgMap.loadOperatorDataSuccess, {
            maxDiffPixels: 500,
        });
    });

    // 【case】非对比非多机see more表格加载
    test('expand_table_when_click_seeMoreCell', async ({ page, operatorPage }) => {
        const { operatorFrame, rankIdSelector } = operatorPage;
        const seeMoreBtn = operatorFrame.getByRole('button', { name: 'See more' }).first();
        const rankIdSelect = new SelectHelpers(page, rankIdSelector, operatorFrame);
        await rankIdSelect.open();
        await rankIdSelect.selectOption('0 0');
        await seeMoreBtn.click();
        await page.mouse.move(0, 0);
        await expect(operatorFrame.locator('.mi-page')).toHaveScreenshot(operatorImgMap.expandOperatorDetailTableDataSuccess, {
            maxDiffPixels: 500,
        });
    });

    // 对比数据
    test('operator_compare_rank', async ({ page, operatorPage }) => {
        const { operatorFrame } = operatorPage;
        await setCompare(page, operatorFrame);
        await operatorFrame.locator('.ant-spin').waitFor({ state: 'hidden' });
        await expect(operatorFrame.locator('.mi-page')).toHaveScreenshot(operatorImgMap.compareRankRes, {
            maxDiffPixels: 500,
        });
        await page.waitForTimeout(2000); // 对比场景需要加延时，确保稳定
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });
});
