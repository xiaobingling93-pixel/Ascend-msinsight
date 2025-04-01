/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { Page, Locator, expect, FrameLocator } from '@playwright/test';

export class Component {
    protected page: Page;
    protected framePage?: FrameLocator;
    protected locator: Locator;

    /**
     * @param page 组件所在的页面对象
     * @param framePage 组件所在的frameLocator
     * @param locator 如何定位组件的根元素，传入字符串通过 getByTestId 获取，或者直接传入一个 locator
     */
    constructor(page: Page, locator: Locator | string = '', framePage?: FrameLocator) {
        this.page = page;
        this.framePage = framePage;

        if (typeof locator === 'string') {
            this.locator = locator ? page.getByTestId(locator) : page.locator('body');
        } else {
            this.locator = locator;
        }
    }

    async isVisible(): Promise<void> {
        await expect(this.locator).toBeVisible();
    }
}
