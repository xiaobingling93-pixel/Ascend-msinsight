/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type { FrameLocator, Locator, Page } from '@playwright/test';
import { FrameworkPage } from './framework';

export class LeaksPage {
    readonly page: Page;
    readonly leaksFrame: FrameLocator;
    readonly threadIdSelector: Locator;
    readonly funcsSelector: Locator;
    readonly deviceIdSelector: Locator;
    readonly typeSelector: Locator;

    constructor(page: Page) {
        this.page = page;
        this.leaksFrame = page.frameLocator('#Leaks');
        this.threadIdSelector = this.leaksFrame.locator('#select-threadId');
        this.funcsSelector = this.leaksFrame.locator('#select-funcName');
        this.deviceIdSelector = this.leaksFrame.locator('#select-deviceId');
        this.typeSelector = this.leaksFrame.locator('#select-type');
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.clickTab('leaks');
    }
}
