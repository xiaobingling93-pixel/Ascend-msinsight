/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type { FrameLocator, Locator, Page } from '@playwright/test';
import { FrameworkPage } from './framework';

export class OperatorPage {
    readonly page: Page;
    readonly operatorFrame: FrameLocator;
    readonly groupIdSelector: Locator;
    readonly hostSelector: Locator;
    readonly rankIdSelector: Locator;
    readonly topSelector: Locator;

    constructor(page: Page) {
        this.page = page;
        this.operatorFrame = page.frameLocator('#Operator');
        this.groupIdSelector = this.operatorFrame.locator('#select-groupId');
        this.hostSelector = this.operatorFrame.locator('#select-host');
        this.rankIdSelector = this.operatorFrame.locator('#select-rankId');
        this.topSelector = this.operatorFrame.locator('#select-top');
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.goto();
        await frameworkPage.clickTab('operator');
    }
}
