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

export class DetailsPage {
    readonly page: Page;
    readonly tabName: TabName = 'details';

    constructor(page: Page) {
        this.page = page;
    }

    get detailsFrame(): FrameLocator {
        return this.page.frameLocator('#Details');
    }

    get mainContent(): Locator {
        return this.detailsFrame.locator('.mi-page');
    }

    get baseInfoContent(): Locator {
        return this.detailsFrame.locator('#baseinfo');
    }

    get coreOccupancyContent(): Locator {
        return this.detailsFrame.locator('#coreOccupancy');
    }

    get coreOccupancyShowAsSelector(): Locator {
        return this.detailsFrame.locator('#core_show_as');
    }

    get rooflineChart(): Locator {
        return this.detailsFrame.getByTestId('rooflineChart');
    }

    get rooflineAdvice(): Locator {
        return this.detailsFrame.getByTestId('rooflineAdvice');
    }

    get ComputeWorkloadChart(): Locator {
        return this.detailsFrame.locator('#ComputeWorkload');
    }

    get computeWorkloadBlockIdSelector(): Locator {
        return this.detailsFrame.locator('#compute_block_id');
    }

    get computeWorkloadAdvice(): Locator {
        return this.detailsFrame.getByTestId('computeWorkloadAdivce');
    }

    get computeWorkloadTable(): Locator {
        return this.detailsFrame.getByTestId('computeWorkloadTable');
    }

    get memoryWorkloadChart(): Locator {
        return this.detailsFrame.locator('#memory');
    }

    get memoryWorkloadAdvice(): Locator {
        return this.detailsFrame.getByTestId('memoryWorkloadAdvice');
    }

    get memoryWorkloadTable(): Locator {
        return this.detailsFrame.getByTestId('memoryWorkloadTable');
    }

    async goto(): Promise<void> {
        const frameworkPage = new FrameworkPage(this.page);
        await frameworkPage.clickTab(this.tabName);
    }

    async mouseOut(): Promise<void> {
        await this.page.mouse.move(0, 0);
    }
}
