/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { test, expect } from '@playwright/test';
import { FilePath } from '@/utils/constants';
import { clearAllData, importData } from '@/utils';

// jupyter 页面正常加载
test('test_jupyterLoaded', async ({ page }) => {
    await page.goto('/');
    const menu = page.frameLocator('#Jupyter').frameLocator('#jupyter').getByLabel('File Browser Section').getByText('test.ipynb');
    await importData(page, FilePath.JUPYTER);
    await menu.waitFor({ state: 'visible' });
    await expect(menu).toBeVisible();
    await clearAllData(page);
});
