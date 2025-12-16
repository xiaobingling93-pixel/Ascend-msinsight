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
