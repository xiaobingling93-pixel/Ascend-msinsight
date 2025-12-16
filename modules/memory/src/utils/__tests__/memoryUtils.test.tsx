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
import { binarySearch } from '../memoryUtils';

const memoryCurveTestData = [
    ['64.79', '4599.74', '8582.00', 'NULL'],
    ['77.20', '4600.05', '8582.00', 'NULL'],
    ['77.49', '4856.06', '8582.00', 'NULL'],
    ['77.59', '5112.06', '8582.00', 'NULL'],
    ['77.91', null, '8824.00', 'NULL'],
    ['77.91', '4856.18', '8824.00', 'NULL'],
    ['78.15', '4920.24', '8824.00', 'NULL'],
    ['78.23', '4664.24', '8824.00', 'NULL'],
    ['78.26', '4889.22', '8824.00', 'NULL'],
    ['78.26', null, '8824.00', 'NULL'],
    ['78.60', '4664.12', '8824.00', 'NULL'],
    ['78.68', '4664.13', '8824.00', 'NULL'],
    ['78.84', '4664.09', '8824.00', 'NULL'],
    ['78.84', null, '8824.00', 'NULL'],
    ['78.84', null, '8824.00', 'NULL'],
    ['78.84', null, '8824.00', 'NULL'],
    ['79.04', null, '8824.00', 'NULL'],
    ['79.11', '4664.09', '8824.00', 'NULL'],
];

const compareFun = (key: number, mid: Array<number | string>): number => key - parseFloat(mid[0] as string);

// 测试不同场景下二分搜索的准确性
it('test function binarySearch with key in the array then return the index otherwise return -1', () => {
    // 测试仅有一个匹配项的场景
    const uniqueKey = 78.15;
    expect(binarySearch(memoryCurveTestData, uniqueKey, compareFun)).toBe(6);
    // 测试所有匹配项的第2个元素均为null的场景
    const itemIsNullKey = 79.04;
    expect(binarySearch(memoryCurveTestData, itemIsNullKey, compareFun)).toBe(-1);
    // 测试有多个匹配项的场景，匹配项需满足第2个元素不为null
    const multiKey = 78.26;
    expect(binarySearch(memoryCurveTestData, multiKey, compareFun)).toBe(8);
    const multiKey2 = 77.91;
    expect(binarySearch(memoryCurveTestData, multiKey2, compareFun)).toBe(5);
    const multiKey3 = 78.84;
    expect(binarySearch(memoryCurveTestData, multiKey3, compareFun)).toBe(12);
    // 测试无匹配项的场景
    const notExistKey = 80;
    expect(binarySearch(memoryCurveTestData, notExistKey, compareFun)).toBe(-1);
    // 测试匹配项是数组第一个或最后一个元素的场景
    const boundaryKey1 = 64.79;
    expect(binarySearch(memoryCurveTestData, boundaryKey1, compareFun)).toBe(0);
    const boundaryKey2 = 79.11;
    expect(binarySearch(memoryCurveTestData, boundaryKey2, compareFun)).toBe(17);
    // 测试数组为空时的场景
    const emptyKey = 1;
    expect(binarySearch([], emptyKey, compareFun)).toBe(-1);
});
