/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { type Locator, type FrameLocator, type Page } from '@playwright/test';
import { FrameworkPage } from './framework';

export class TimelinePage {
    readonly page: Page;
    readonly timelineFrame: FrameLocator;
    readonly fullPage: Locator;
    readonly mainContainer: Locator; // 主页面（不包括底部面板）
    readonly unitWrapperScroller: Locator; // 泳道容器（不包括置顶泳道）
    readonly markerBtn: Locator;
    readonly filterBtn: Locator;
    readonly searchBtn: Locator;
    readonly flowBtn: Locator;
    readonly resetBtn: Locator;
    readonly zoomInBtn: Locator;
    readonly zoomOutBtn: Locator;
    readonly drawerBtn: Locator; // 底部面板收缩按钮
    readonly bottomPanel: Locator; // 底部面板
    readonly selectFilterType: Locator;
    readonly selectOptionFilterType: Locator;
    readonly selectFilterContent: Locator;
    readonly openInWindows: Locator;

    constructor(page: Page) {
        this.page = page;
        this.timelineFrame = page.frameLocator('#Timeline');
        this.fullPage = this.timelineFrame.locator('#root');
        this.mainContainer = this.timelineFrame.locator('#main-container');
        this.unitWrapperScroller = this.timelineFrame.locator('#unitWrapperScroller');
        this.markerBtn = this.timelineFrame.getByTestId('tool-marker');
        this.filterBtn = this.timelineFrame.getByTestId('tool-filter');
        this.searchBtn = this.timelineFrame.getByTestId('tool-search');
        this.flowBtn = this.timelineFrame.getByTestId('tool-flow');
        this.resetBtn = this.timelineFrame.getByTestId('tool-reset');
        this.zoomInBtn = this.timelineFrame.getByTestId('tool-zoom-in');
        this.zoomOutBtn = this.timelineFrame.getByTestId('tool-zoom-out');
        this.drawerBtn = this.timelineFrame.getByTestId('drawer-btn').nth(2);
        this.bottomPanel = this.timelineFrame.locator('.bottomPanelContainer');
        this.selectFilterType = this.timelineFrame.locator('#select-filter-type');
        this.selectOptionFilterType = this.timelineFrame.getByTestId('select-options-filter-type');
        this.selectFilterContent = this.timelineFrame.locator('#select-filter-content');
        this.openInWindows = this.timelineFrame.getByRole('button', { name: 'Open in Find Window' });
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
    readonly selectSystemView: Locator;

    constructor(page: Page) {
        super(page);
        this.selectSystemView = this.timelineFrame.locator('#select-system-view');
    }

    async goto(): Promise<void> {
        await this.drawerBtn.click();
        await this.timelineFrame.getByRole('tab', { name: 'System View' }).click();
    }
}
