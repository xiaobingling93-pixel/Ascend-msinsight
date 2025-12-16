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
