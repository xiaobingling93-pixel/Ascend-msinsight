/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
