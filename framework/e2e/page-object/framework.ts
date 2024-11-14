/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { expect, type Locator, type Page } from '@playwright/test';

export type TabName = 'timeline' | 'memory' | 'operator' | 'summary' | 'communication' | 'Source' | 'details' | 'trace' | 'cache';
export class FrameworkPage {
    readonly page: Page;
    // 顶部功能按钮
    readonly deleteAllBtn: Locator;
    readonly importDataBtn: Locator;
    readonly switchThemeBtn: Locator;
    readonly switchLanguageBtn: Locator;
    readonly helpInfoBtn: Locator;

    // 左侧项目列表
    readonly projectList: Locator;

    // 模块标签
    readonly timelineTab: Locator;
    readonly memoryTab: Locator;
    readonly operatorTab: Locator;
    readonly summaryTab: Locator;
    readonly communicationTab: Locator;
    readonly sourceTab: Locator;
    readonly detailsTab: Locator;

    // 删除全部弹窗
    readonly deleteAllDialog: Locator;
    readonly deleteAllConfirmBtn: Locator;
    readonly deleteAllCancelBtn: Locator;

    // 帮助信息弹窗
    readonly helpInfoDialog: Locator;

    readonly loadingDialog: Locator;

    constructor(page: Page) {
        this.page = page;
        this.deleteAllBtn = page.locator('.btn-delete');
        this.importDataBtn = page.locator('.btn-import');
        this.switchThemeBtn = page.locator('.switch-theme');
        this.switchLanguageBtn = page.getByTestId('switch-lng');
        this.helpInfoBtn = page.getByTestId('help-icon');
        this.timelineTab = page.getByRole('menuitem', { name: 'Timeline' });
        this.memoryTab = page.getByRole('menuitem', { name: 'Memory' });
        this.operatorTab = page.getByRole('menuitem', { name: 'Operator' });
        this.summaryTab = page.getByRole('menuitem', { name: 'Summary' });
        this.communicationTab = page.getByRole('menuitem', { name: 'Communication' });
        this.sourceTab = page.getByRole('menuitem', { name: 'Source' });
        this.detailsTab = page.getByRole('menuitem', { name: 'Details' });
        this.deleteAllDialog = page.getByLabel('Delete All');
        this.deleteAllConfirmBtn = page.getByRole('button', { name: 'Confirm' });
        this.deleteAllCancelBtn = page.getByRole('button', { name: 'Cancel' });
        this.projectList = page.getByRole('tree');
        this.helpInfoDialog = page.getByLabel('About MindStudio Insight');
        this.loadingDialog = page.locator('.el-loading-mask');
    }

    async goto(): Promise<void> {
        await this.page.goto('/');
    }

    async clickTab(tabName: TabName): Promise<void> {
        await this[`${tabName}Tab`].click();
    }

    async goToTab(tabName: TabName): Promise<void> {
        const tab = this.page.getByRole('menuitem', {name: tabName});
        await expect(tab).toBeVisible();
        await tab.click();
    }

    async mouseOut(): Promise<void> {
        await this.page.mouse.move(0, 0);
    }
}

export class FileExploreDialogPage {
    readonly page: Page;
    readonly mainDialog: Locator;
    readonly cancelBtn: Locator;
    readonly confirmBtn: Locator;
    readonly closeBtn: Locator;
    readonly input: Locator;

    constructor(page: Page) {
        this.page = page;
        this.mainDialog = page.getByRole('dialog', { name: 'File Explorer' });
        this.cancelBtn = page.getByRole('button', { name: 'Cancel' });
        this.confirmBtn = page.getByRole('button', { name: 'Confirm' });
        this.closeBtn = page.getByLabel('Close this dialog');
        this.input = page.getByPlaceholder('Enter the file path and press');
    }
}
