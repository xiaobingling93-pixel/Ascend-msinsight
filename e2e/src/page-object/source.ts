/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type { FrameLocator, Page } from '@playwright/test';
import { FrameworkPage, TabName } from './framework';
import { Locator } from '@playwright/test';

export class SourcePage {
    readonly page: Page;
    readonly tabName: TabName = 'Source';

    constructor(page: Page) {
        this.page = page;
    }

    get sourceFrame(): FrameLocator {
        return this.page.frameLocator('#Source');
    }

    get mainContent(): Locator {
        return this.sourceFrame.locator('#hotMethod');
    }

    get coreSelector(): Locator {
        return this.sourceFrame.locator('#coreSelect');
    }

    get sourceSelector(): Locator {
        return this.sourceFrame.locator('#sourceSelect');
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.clickTab(this.tabName);
    }

    async mouseOut(): Promise<void> {
        await this.page.mouse.move(0, 0);
    }
}
