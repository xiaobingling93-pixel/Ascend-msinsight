/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { expect, test, WebSocket } from '@playwright/test';
import { FrameworkPage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener, waitForResponse } from '@/utils';
import { FilePath } from '@/utils/constants';

let ws: Promise<WebSocket>;

test.describe('Framework', () => {
    test.beforeEach(async ({ page }) => {
        ws = setupWebSocketListener(page);

        const frameworkPage = new FrameworkPage(page);
        await frameworkPage.goto();
        await clearAllData(page);
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });

    // 切换主题
    test('toggle theme', async ({ page }) => {
        const frameworkPage = new FrameworkPage(page);
        await frameworkPage.switchThemeBtn.click();
        await page.mouse.move(0, 0);
        await expect(page).toHaveScreenshot('theme.png', { maxDiffPixels: 500 });
    });

    // 切换中英文
    test('toggle language', async ({ page }) => {
        const frameworkPage = new FrameworkPage(page);
        await frameworkPage.switchLanguageBtn.click();
        await page.mouse.move(0, 0);
        await expect(page).toHaveScreenshot('language.png', { maxDiffPixels: 500 });
    });

    // 查看帮助信息
    test('view help info', async ({ page }) => {
        const frameworkPage = new FrameworkPage(page);
        const { helpInfoDialog, shortcutsDialog } = frameworkPage;
        await frameworkPage.helpInfoBtn.click();
        await page.getByText('Keyboard shortcuts').click();
        await expect(shortcutsDialog).toBeVisible();
        await page.getByLabel('Close', { exact: true }).click();

        await frameworkPage.helpInfoBtn.click();
        await page.getByText('About').click();
        await expect(helpInfoDialog).toBeVisible();
    });

    // 设置项目为基线
    test('set project as baseline', async ({ page  }) => {
        const frameworkPage = new FrameworkPage(page);

        const waitFirstRes = waitForResponse(await ws, (res) => res?.event === 'allPagesSuccess');
        await importData(page, FilePath.DB_2025330);
        await waitFirstRes;
        const waitSecondRes = waitForResponse(await ws, (res) => res?.event === 'allPagesSuccess');
        await importData(page, FilePath.DB_memory);
        await waitSecondRes;
        await frameworkPage.projectList.getByText(FilePath.DB_2025330, { exact: true }).click({
            button: 'right',
        });
        await page.getByText('Set as Baseline Data').click();
        await page.mouse.move(0, 0);
        await expect(frameworkPage.projectList).toHaveScreenshot('set-project-baseline.png', { maxDiffPixels: 500 });
        await page.waitForTimeout(2000);
    });
});
