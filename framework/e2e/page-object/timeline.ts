/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { expect, type Locator, type FrameLocator, type Page } from '@playwright/test';
import { FrameworkPage } from './framework';

export class TimelinePage {
    readonly page: Page;
    readonly timelineFrame: FrameLocator;
    readonly markerBtn: Locator;
    readonly filterBtn: Locator;
    readonly searchBtn: Locator;
    readonly flagBtn: Locator;
    readonly resetBtn: Locator;
    readonly zoomInBtn: Locator;
    readonly zoomOutBtn: Locator;
    readonly drawerBtn: Locator; // 底部面板收缩按钮
    readonly bottomPanel: Locator; // 底部面板

    constructor(page: Page) {
        this.page = page;
        this.timelineFrame = page.frameLocator('#Timeline');
        this.searchBtn = this.timelineFrame.getByTestId('tool-search');
        this.drawerBtn = this.timelineFrame.getByTestId('drawer-btn').nth(1);
        this.bottomPanel = this.timelineFrame.locator('.bottomPanelContainer');
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.goto();
        await frameworkPage.clickTab('timeline');
    }

    async expandUnit(unitLocator: Locator): Promise<void> {
        const expandBtn = unitLocator.getByTestId('expand-btn');
        const isExpanded = await expandBtn.evaluate((el) => el.classList.contains('insight-unit-expanded'));
        if (isExpanded) {
            return;
        }
        await expandBtn.click();
    }

    async collapseUnit(unitLocator: Locator): Promise<void> {
        const expandBtn = unitLocator.getByTestId('expand-btn');
        const isFold = await expandBtn.evaluate((el) => el.classList.contains('insight-unit-fold'));
        if (isFold) {
            return;
        }
        await expandBtn.click();
    }

    async clickMenu(clickUnit: Locator, timelineFrame: FrameLocator, option: string): Promise<void> {
        await clickUnit.click({ button: 'right' });
        const options = timelineFrame.locator('.menu-item');
        await options.getByText(option).click();
    }
}

export class SystemView extends TimelinePage {
    constructor(page: Page) {
        super(page);
    }

    async goto(): Promise<void> {
        await this.drawerBtn.click();
        await this.timelineFrame.getByRole('tab', { name: 'System View' }).click();
    }
}
