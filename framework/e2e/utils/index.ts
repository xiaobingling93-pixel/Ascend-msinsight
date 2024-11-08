/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { expect, type Page } from '@playwright/test';
import { FrameworkPage, FileExploreDialogPage } from '../page-object';
import { FilePath } from './constants';

// 导入数据
export async function importData(page: Page, filePath: FilePath = FilePath.TEXT): Promise<void> {
    const frameworkPage = new FrameworkPage(page);
    const fileExploreDialogPage = new FileExploreDialogPage(page);
    const { importDataBtn, projectList } = frameworkPage;
    const { mainDialog, input, confirmBtn } = fileExploreDialogPage;

    // 点击“导入数据”按钮
    await importDataBtn.click();

    // 确认弹窗已打开
    await expect(mainDialog).toBeVisible();

    await input.fill(filePath);
    await input.press('Enter');

    // 点击“确认”按钮
    await confirmBtn.click();
    const projectListItemCount = await projectList.getByRole('treeitem').count();
    expect(projectListItemCount).toBeGreaterThan(0);
    const currentProject = projectList.getByText(filePath, { exact: true }).first();
    await expect(currentProject).toBeVisible();
}

// 清除数据
export async function clearAllData(page: Page): Promise<void> {
    const frameworkPage = new FrameworkPage(page);
    const { deleteAllBtn, deleteAllDialog, deleteAllConfirmBtn, projectList } = frameworkPage;
    const isDisabled = await deleteAllBtn.evaluate((el) => el.classList.contains('disabled'));

    if (isDisabled) {
        return;
    }
    await deleteAllBtn.click();
    await expect(deleteAllDialog).toBeVisible();
    await deleteAllConfirmBtn.click();

    const noData = projectList.getByText('No Data');
    await noData.waitFor({ state: 'visible' });
    await expect(noData).toBeVisible();
}

// 等待ws返回指定数据
export async function waitForWebSocketEvent<T>(page: Page, matchCondition: (payload: any) => boolean): Promise<T> {
    return new Promise((resolve, reject) => {
        page.on('websocket', (ws) => {
            ws.on('framereceived', (event) => {
                if (typeof event.payload !== 'string') {
                    return;
                }
                try {
                    const res = JSON.parse(event.payload);
                    if (matchCondition(res)) {
                        resolve(res);
                    }
                } catch (error) {
                    reject(new Error(`WebSocket 消息解析失败: ${error}`));
                }
            });
        });
    });
}
