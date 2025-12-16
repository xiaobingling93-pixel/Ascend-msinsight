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

import { expect } from '@playwright/test';
import { Component } from './Component';

export class InputHelpers extends Component {
    // 设置输入框的值
    async setValue(text: string): Promise<void> {
        await this.locator.fill(text);
    }

    // 清空输入框的值
    async clear(): Promise<void> {
        await this.locator.fill('');
    }

    // 获取当前输入框的值
    async getValue(): Promise<string> {
        return await this.locator.inputValue();
    }

    // 按下键盘键
    async press(key: string): Promise<void> {
        return await this.locator.press(key);
    }

    // 断言输入框的值是否为指定文本
    async expectValueToBe(expectedText: string): Promise<void> {
        await expect(this.locator).toHaveValue(expectedText);
    }
}
