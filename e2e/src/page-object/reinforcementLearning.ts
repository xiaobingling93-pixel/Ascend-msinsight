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

export class RLPage {
    readonly page: Page;
    readonly rLFrame: FrameLocator;

    constructor(page: Page) {
        this.page = page;
        this.rLFrame = page.frameLocator('#RL');
        this.taskTraceTimelineContent = this.rLFrame.getByTestId('task-trace-timeline');
        this.taskExecutionTimeline = this.rLFrame.locator('#select-host');
    }

    async goto(val): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        if(val !== 'RL') {
            await frameworkPage.goto();
        }
        await frameworkPage.clickTab(val);
    }
}
