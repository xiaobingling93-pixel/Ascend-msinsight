/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

// 导入要测试的函数
import { anonymousString, handlerEmptyString } from '../string';

describe('anonymousString', () => {
    it('should return the original string if it is undefined or less than 3 characters', () => {
        expect(anonymousString('')).toBe('');
        expect(anonymousString('a')).toBe('a');
        expect(anonymousString('ab')).toBe('ab');
    });

    it('should return a string with the middle third replaced by asterisks', () => {
        expect(anonymousString('abcdef')).toBe('ab**ef');
        expect(anonymousString('abcdefghi')).toBe('abc***ghi');
    });
});

describe('handlerEmptyString', () => {
    it('should return the default value if the string is undefined or empty', () => {
        expect(handlerEmptyString('', 'default')).toBe('default');
    });

    it('should return the original string if it is not empty', () => {
        expect(handlerEmptyString('hello', 'default')).toBe('hello');
    });
});
