/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { Component } from './Component';

export class SelectHelpers extends Component {
    // 点击输入框以打开下拉菜单
    async open(): Promise<void> {
        const selector = this.locator.locator('xpath=../..');
        await selector?.click();
    }

    async setValue(value: string): Promise<void> {
        await this.locator.fill(value);
    }

    async selectOption(optionText: string): Promise<void> {
        const _page = this.framePage ?? this.page;
        const selectorId = await this.locator.getAttribute('id');

        const selectOptions = _page.locator(`#${selectorId}_list + div`);
        await selectOptions.waitFor();
        const option = selectOptions.locator(`.ant-select-item-option[title='${optionText}']`);

        await option.click();
    }

    async getValue(): Promise<string> {
        return await this.locator.locator('xpath=..').locator('xpath=..').innerText();
    }
}
