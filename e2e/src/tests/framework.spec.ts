/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test, expect } from '@playwright/test';
import { FrameworkPage } from '@/page-object';

test.describe('Framework', () => {
    test.beforeEach(async ({ page }) => {
        const frameworkPage = new FrameworkPage(page);
        await frameworkPage.goto();
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
});
