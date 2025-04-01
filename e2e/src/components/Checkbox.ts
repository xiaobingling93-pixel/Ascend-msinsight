/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { Component } from './Component';

export class CheckboxHelpers extends Component {
    // 点击复选框
    async click(): Promise<void> {
        const checkbox = this.locator.locator('xpath=../..');
        await checkbox?.click();
    }

    // 获取点击情况
    async isChecked(): Promise<boolean> {
        return await this.locator.locator('xpath=../..').isChecked();
    }
}
