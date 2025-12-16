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
        this.leaksFrame = page.frameLocator('#MemScope');
        this.threadIdSelector = this.leaksFrame.locator('#select-threadId');
        this.funcsSelector = this.leaksFrame.locator('#select-funcName');
        this.deviceIdSelector = this.leaksFrame.locator('#select-deviceId');
        this.typeSelector = this.leaksFrame.locator('#select-type');
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.clickTab('MemScope');
    }
}
