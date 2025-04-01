/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type { FrameLocator, Locator, Page } from '@playwright/test';
import { FrameworkPage } from './framework';

export class MemoryPage {
    readonly page: Page;
    readonly memoryFrame: FrameLocator;
    readonly hostSelector: Locator;
    readonly rankIdSelector: Locator;
    readonly groupIdSelector: Locator;
    readonly graphIdSelector: Locator;
    readonly nameInputor: Locator;
    readonly minSizeInputor: Locator;
    readonly maxSizeInputor: Locator;
    readonly isOnlyShowAllocatedOrReleasedWithinIntervalChecker: Locator;

    constructor(page: Page) {
        this.page = page;
        this.memoryFrame = page.frameLocator('#Memory');
        this.hostSelector = this.memoryFrame.locator('#select-host');
        this.rankIdSelector = this.memoryFrame.locator('#select-rankId');
        this.groupIdSelector = this.memoryFrame.locator('#select-groupId');
        this.graphIdSelector = this.memoryFrame.locator('#select-graphId');
        this.nameInputor = this.memoryFrame.locator('#input-name');
        this.minSizeInputor = this.memoryFrame.locator('#input-minSize');
        this.maxSizeInputor = this.memoryFrame.locator('#input-maxSize');
        this.isOnlyShowAllocatedOrReleasedWithinIntervalChecker =
            this.memoryFrame.locator('#input-onlyShowAllocatedOrReleased');
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.goto();
        await frameworkPage.clickTab('memory');
    }
}
