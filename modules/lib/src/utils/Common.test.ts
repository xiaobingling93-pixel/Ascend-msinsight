/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { safeStr } from './Common';

// 测试字符串转义方法
describe('test function safeStr', () => {
    it('test input string', () => {
        const unSafeText = 'This is unsafe / " \' & <div>div</div>';
        expect(safeStr(unSafeText)).toBe('This is unsafe &#x2F; &quot; &#39; &amp; &lt;div&gt;div&lt;&#x2F;div&gt;');
    });

    it('test input number', () => {
        const num = 100;
        expect(safeStr(num)).toBe('100');
    });

    it('test ignore', () => {
        const unSafeText = 'This is unsafe / " \' & <div>div</div>';
        expect(safeStr(unSafeText, '')).toBe('This is unsafe &#x2F; &quot; &#39; &amp; &lt;div&gt;div&lt;&#x2F;div&gt;');
        expect(safeStr(unSafeText, '<')).toBe('This is unsafe &#x2F; &quot; &#39; &amp; <div&gt;div<&#x2F;div&gt;');
        expect(safeStr(unSafeText, '>')).toBe('This is unsafe &#x2F; &quot; &#39; &amp; &lt;div>div&lt;&#x2F;div>');
        expect(safeStr(unSafeText, '&')).toBe('This is unsafe &#x2F; &quot; &#39; & &lt;div&gt;div&lt;&#x2F;div&gt;');
        expect(safeStr(unSafeText, '\'')).toBe('This is unsafe &#x2F; &quot; \' &amp; &lt;div&gt;div&lt;&#x2F;div&gt;');
        expect(safeStr(unSafeText, '"')).toBe('This is unsafe &#x2F; " &#39; &amp; &lt;div&gt;div&lt;&#x2F;div&gt;');
        expect(safeStr(unSafeText, '/')).toBe('This is unsafe / &quot; &#39; &amp; &lt;div&gt;div&lt;/div&gt;');
    });
});