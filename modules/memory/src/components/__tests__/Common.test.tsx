/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import * as React from 'react';
import { render, screen } from '@testing-library/react';
import { Label, safeStr } from '../Common';

/**
 * 生成指定长度的随机字符串，使用字符串列表和随机数对应的方式进行匹配
 * @param length 字符串长度
 * @returns 随机字符串
 */
const generateRandomString = (length: number): string => {
    let result = '';
    const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    for (let i = 0; i < length; i++) {
        result += characters.charAt(Math.floor(Math.random() * characters.length));
    }
    return result;
};

// 测试Label组件是否能正确显示随机文本
it('test function Label with random string then show correct text', () => {
    const randomName = generateRandomString(10);
    render(<Label name={randomName} />);
    expect(screen.getByText(`${randomName} :`)).toBeDefined();
});

// 测试字符串转义以及ignore参数
it('test function safeStr with "<" or ">" in string should be escaped', () => {
    const unSafeText = 'This is unsafe <div>div</div>';
    expect(safeStr(unSafeText)).toBe('This is unsafe &lt;div&gt;div&lt;/div&gt;');
    expect(safeStr(unSafeText, '<')).toBe('This is unsafe <div&gt;div</div&gt;');
    expect(safeStr(unSafeText, '>')).toBe('This is unsafe &lt;div>div&lt;/div>');
});
