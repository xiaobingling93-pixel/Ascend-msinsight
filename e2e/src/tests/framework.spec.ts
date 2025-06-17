/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { expect, test } from '@playwright/test';
import { FrameworkPage } from '@/page-object';
import { clearAllData, importData } from '@/utils';
import { FilePath } from '@/utils/constants';

test.describe('Framework', () => {
    test.beforeEach(async ({ page }) => {
        const frameworkPage = new FrameworkPage(page);
        await frameworkPage.goto();
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
        await importData(page, FilePath.DB_2025330);
        await page.waitForTimeout(5000); // 等待前面的项目加载完成
        await importData(page, FilePath.DB_memory);
        await frameworkPage.projectList.getByText(FilePath.DB_2025330, { exact: true }).click({
            button: 'right',
        });
        await page.getByText('Set as Baseline Data').click();
        await page.mouse.move(0, 0);
        await expect(frameworkPage.projectList).toHaveScreenshot('set-project-baseline.png', { maxDiffPixels: 500 });
    });
});
