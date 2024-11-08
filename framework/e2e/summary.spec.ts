/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { test as baseTest, expect } from '@playwright/test';
import { SummaryPage, FrameworkPage } from './page-object';
import { clearAllData, importData } from './utils';

interface TestFixtures {
    summaryPage: SummaryPage;
}
const test = baseTest.extend<TestFixtures>({
    summaryPage: async ({ page }, use) => {
        const summaryPage = new SummaryPage(page);
        await use(summaryPage);
    },
});

test.describe('Summary', () => {
    test.beforeEach(async ({ page, summaryPage }) => {
        const { loadingDialog } = new FrameworkPage(page);
        const { fullmask } = summaryPage;
        await page.goto('/');
        await importData(page);
        await summaryPage.goto();
        if (await loadingDialog.count()) {
            await loadingDialog.waitFor({ state: 'attached' });
        }
        if (await fullmask.count()) {
            await fullmask.waitFor({ state: 'hidden' });
        }
    });

    // 基本数据是否加载
    test('load success', async ({ page, summaryPage }) => {
        const { summaryFrame } = summaryPage;
        const filePathDom = summaryFrame.locator('.content').first();
        await filePathDom.waitFor({ state: 'visible' });
        const filePath = await filePathDom.innerText();
        expect(filePath.length > 0).toBe(true);
    });

    // 配置并行策略
    test('configure parallel strategy', async ({ page, summaryPage }) => {
        const { summaryFrame, btnGenerate, configureParallel } = summaryPage;

        if (await btnGenerate.isDisabled()) {
        } else {
            await configureParallel(
                summaryFrame,
                btnGenerate,
                { algorithm: 'Megatron-LM(tp-dp-pp)', ppSize: 2, tpSize: 2, dpSize: 2 },
            );
            await page.mouse.move(0, 0);
            await expect(summaryFrame.getByTestId('parallelRankContainer')).toHaveScreenshot('parallelRank.png');
        }
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });
});
