/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
