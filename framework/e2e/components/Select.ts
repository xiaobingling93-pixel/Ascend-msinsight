/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { expect } from '@playwright/test';
import { Component } from './Component';

export class SelectHelpers extends Component {
    // 点击输入框以打开下拉菜单
    async open(): Promise<void> {
        const selector = this.locator.locator('xpath=../..');
        await selector?.click();
    }

    async selectOption(optionText: string): Promise<void> {
        const _page = this.framePage ?? this.page;
        const optionsDropdown = _page.locator('.ant-select-dropdown');
        await optionsDropdown.waitFor();
        const selectorId = await this.locator.getAttribute('id');
        const optionLocator = _page.locator(`#${selectorId}_list + div .ant-select-item-option`);
        const option = optionLocator.getByText(optionText);

        await option.click();
    }

    async getValue(): Promise<string> {
        return await this.locator.innerText();
    }
}
