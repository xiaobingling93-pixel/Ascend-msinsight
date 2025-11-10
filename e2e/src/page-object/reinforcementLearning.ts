/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type { FrameLocator, Locator, Page } from '@playwright/test';
import { FrameworkPage } from './framework';

export class RLPage {
    readonly page: Page;
    readonly rLFrame: FrameLocator;

    constructor(page: Page) {
        this.page = page;
        this.rLFrame = page.frameLocator('#RL');
        this.taskExecutionTimeline = this.rLFrame.locator('#select-host');
    }

    async goto(val): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        if(val !== 'RL'){
            await frameworkPage.goto();
        }
        await frameworkPage.clickTab(val);
    }
}
