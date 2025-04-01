/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
