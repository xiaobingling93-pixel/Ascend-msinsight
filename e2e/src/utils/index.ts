/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { expect, type FrameLocator, type Page, WebSocket } from '@playwright/test';
import { FrameworkPage, FileExploreDialogPage } from '../page-object';
import { FilePath, WEBSOCKET_URL } from './constants';

let iterationNum = 0;
// 导入数据
export async function importData(page: Page, filePath: FilePath = FilePath.TEXT): Promise<void> {
    if (iterationNum > 3) {
        return;
    }
    iterationNum++;
    try {
        const frameworkPage = new FrameworkPage(page);
        const fileExploreDialogPage = new FileExploreDialogPage(page);
        const { importDataBtn, projectList } = frameworkPage;
        const { mainDialog, input, confirmBtn, fileTree } = fileExploreDialogPage;

        // 点击“导入数据”按钮
        await importDataBtn.click();

        // 确认弹窗已打开
        await expect(mainDialog).toBeVisible();
        const emptyBlock = fileTree.locator('.el-tree__empty-block');
        await emptyBlock.waitFor({ state: 'hidden', timeout: 2000 });
        await input.waitFor({ state: 'visible', timeout: 2000 });
        await input.click();
        await input.fill(filePath);
        await input.press('Enter');
        // 点击“确认”按钮
        await confirmBtn.click();
        await mainDialog.waitFor({ state: 'hidden', timeout: 2000 });
        const currentProject = projectList.getByText(filePath, { exact: true }).first();
        await currentProject.waitFor({ state: 'visible' });
        await expect(currentProject).toBeVisible();
    } catch {
        await page.reload();
        await importData(page, filePath);
    }
    iterationNum = 0;
}

// 清除数据
export async function clearAllData(page: Page): Promise<void> {
    const frameworkPage = new FrameworkPage(page);
    const { settingsBtn, deleteAllBtn, deleteAllDialog, deleteAllConfirmBtn, projectList } = frameworkPage;
    const isSettingsBtnDisabled = await settingsBtn.evaluate((el) => el.classList.contains('disabled'));

    if (isSettingsBtnDisabled) {
        return;
    }
    await settingsBtn.click();

    const isDeleteBtnDisabled = await deleteAllBtn.evaluate((el) => el.classList.contains('disabled'));

    if (isDeleteBtnDisabled) {
        return;
    }
    await deleteAllBtn.click();
    await expect(deleteAllDialog).toBeVisible();
    await deleteAllConfirmBtn.click();

    const checkbtn = page.locator('.dragContainer').first().getByText('All');
    await checkbtn.waitFor({ state: 'hidden' });
    const elementText = await projectList.textContent();
    await expect(elementText?.trim()).toBe('');
}

// 等待ws返回指定数据
export async function waitForWebSocketEvent<T>(page: Page, matchCondition: (payload: any) => boolean): Promise<T> {
    return new Promise((resolve, reject) => {
        page.on('websocket', (ws) => {
            const url = ws.url();
            if (url !== WEBSOCKET_URL) {
                return;
            }
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

// 监听 websocket 连接，返回 ws 实例
export function setupWebSocketListener(page: Page): Promise<WebSocket> {
    return new Promise((resolve) => {
        page.on('websocket', (ws) => {
            const url = ws.url();
            if (url === WEBSOCKET_URL) {
                resolve(ws);
            }
        });
    });
}

// 使用 ws 实例监听特定接口返回
export function waitForResponse(ws: WebSocket, matchCondition: (payload: any) => boolean): Promise<any> {
    return new Promise((resolve, reject) => {
        const frameHandler = (event: any): void => {
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
        };

        ws.on('framereceived', frameHandler);
    });
}

export async function setCompare(
    page: Page,
    frame: FrameLocator,
    {
        baseline,
        comparison,
    }: {
        baseline: string;
        comparison: string;
    } = { baseline: FilePath.TEXT_RANK_0, comparison: FilePath.TEXT_RANK_1 },
): Promise<void> {
    const frameworkPage = new FrameworkPage(page);
    const rank1 = frameworkPage.getRankLocator(baseline);
    await rank1.click({ button: 'right' });
    const setBaselineBtn = frameworkPage.page.getByText('Set as Baseline Data');
    await setBaselineBtn.click();
    const rank2 = frameworkPage.getRankLocator(comparison);
    await rank2.click({ button: 'right' });
    const setComparisonBtn = frameworkPage.page.getByText('Set as Comparison Data');
    await setComparisonBtn.click();
    await frame.getByText('loading').first().waitFor({ state: 'hidden' });
    await frameworkPage.mouseOut();
}

export async function waitForAllParsed(page: Page, tabPage: any): Promise<void> {
    const frameworkPage = new FrameworkPage(page);
    await frameworkPage.clickTab('timeline');
    const timelineFrame = page.frameLocator('#Timeline');
    const parsingProgress = timelineFrame.locator('.unit-info .ant-progress').first();
    await parsingProgress.waitFor({ state: 'hidden' });
    await tabPage.goto();
}
