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
import { getDuration, getReadableMem, getTimestamp, toLocalTimeString } from '../humanReadable';
import type { TimeOptions } from '../adaptTimeForLength';

describe('getDuration test', () => {
    it('basic integer test group', () => {
        // basic test case
        expect(getDuration(200)).toBe('200ns');

        // medium integer test case
        expect(getDuration(2e13)).toBe('5hour 33min 20s');

        // max integer test case
        expect(getDuration(Number.MAX_SAFE_INTEGER)).toBe('2501hour 59min 59s 254ms 740μs 991ns');

        // 0 test case
        expect(getDuration(0)).toBe('0ns');

        // negative integer test case
        expect(getDuration(-1)).toBe('-1ns');

        // medium negative integer test case
        expect(getDuration(-3.33e10)).toBe('-33s -300ms');

        // max negative integer test case
        expect(getDuration(Number.MIN_SAFE_INTEGER)).toBe('-2501hour -59min -59s -254ms -740μs -991ns');
    });

    it('basic double test group', () => {
        // basic double test case
        expect(getDuration(1.1)).toBe('1.1ns');

        // long double test case
        expect(getDuration(1.234567892021)).toBe('1.2ns');

        // max double test case
        expect(getDuration(Number.MAX_VALUE)).toBe('4.993592041284209e+295hour 56min 0s 424ms 848μs 368ns');

        // basic negative double test case
        expect(getDuration(-0.315)).toBe('-0.3ns');

        // long negative double test case
        expect(getDuration(Number.MIN_VALUE)).toBe('0.0ns');

        // biggest negative double test case
        expect(getDuration(-Number.MAX_VALUE)).toBe('-4.993592041284209e+295hour -56min 0s -424ms -848μs -368ns');
    });

    it('basic error test group', () => {
        // infinity test case
        expect(getDuration(Infinity)).toBe('NaN');

        // negative infinity test case
        expect(getDuration(Number.NEGATIVE_INFINITY)).toBe('NaN');

        // NaN test case
        expect(getDuration(parseInt('test case'))).toBe('NaN');
    });

    it('advanced integer test group', () => {
        // adapt for text width test case
        expect(getDuration(Number.MAX_SAFE_INTEGER, { maxChars: 100 / 16 })).toBe('2501.9hour');

        // adapt for text width test case 2
        expect(getDuration(Number.MAX_SAFE_INTEGER, { maxChars: 100 / 1 })).toBe('2501hour 59min 59s 254ms 740μs 991ns');

        // adapt for text width test case 3
        expect(getDuration(Number.MAX_SAFE_INTEGER, { maxChars: 300 / 16 })).toBe('2501hour 59.9min');

        // adapt for oversize text width test case
        expect(getDuration(Number.MAX_SAFE_INTEGER, { maxChars: 1 / 100 })).toBe('2501.9hour');

        // segments text units test case
        expect(getDuration(Number.MAX_SAFE_INTEGER, { segments: 3 })).toBe('2501hour 59min 59.2s');

        // long span segments test case
        expect(getDuration(3.000000003e10, { segments: 1 })).toBe('30.0s');
    });

    it('advanced double test group', () => {
        expect(getDuration(2553709970.279379, { maxChars: 70 / 12 })).toBe('2.5s');
    });

    it('complex test group', () => {
        // all parameterts test case
        expect(getDuration(1, { precision: 'hour', segments: 10, maxChars: 0 })).toBe('1hour');

        // all parameterts test case
        expect(getDuration(Number.MAX_VALUE, { precision: 'min', segments: 10, maxChars: 0 })).toBe('2.9961552247705265e+306hour 8min');
    });
});

describe('getTimestamp', () => {
    it('basic integer test group', () => {
        // begin at ms simple integer test case
        expect(getTimestamp(123456789)).toBe('34:17:36.789');

        // begin at ms max integer test case
        expect(getTimestamp(Number.MAX_SAFE_INTEGER)).toBe('2501999792:59:00.991');

        // begin at ns max integer test case
        expect(getTimestamp(Number.MAX_SAFE_INTEGER, { precision: 'ns' })).toBe('2501:59:59.254.740.991');

        // begin at hour max integer test case
        expect(getTimestamp(Number.MAX_SAFE_INTEGER, { precision: 'hour' })).toBe('9007199254740991');

        // begin at precision max integer test case
        expect(getTimestamp(Number.MAX_SAFE_INTEGER, { precision: 'min' })).toBe('150119987579016:31');

        expect(getTimestamp(-Number.MAX_SAFE_INTEGER, { precision: 'min' })).toBe('-150119987579016:31');

        expect(getTimestamp(-6107545, { precision: 'ns' })).toBe('-00:00.006.107.545');
    });

    it('advanced integer test group', () => {
        // segments to s test case
        expect(getTimestamp(12345, { segments: 4 })).toBe('00:00.000.012');

        // segments to precision test case
        expect(getTimestamp(123456, { segments: 5 })).toBe('00:00.000.123.456');

        // small input value test case
        expect(getTimestamp(1234, { segments: 3 })).toBe('00:00.000');

        // segments max test case
        expect(getTimestamp(1, { segments: Number.MAX_VALUE })).toBe('00:00.000.000.001');

        // default timestamp test case
        expect(getTimestamp(360000)).toBe('06:00.000');
    });

    it('complex test group', () => {
        // all parameterts test case
        expect(getTimestamp(1, { precision: 'hour', segments: 10, maxChars: 0 })).toBe('01');

        // all parameterts test case
        expect(getTimestamp(Number.MAX_VALUE, { precision: 'min', segments: 10, maxChars: 0 })).toBe('2.9961552247705265e+306:08');
    });

    it('when segment is undifine return 000.001', () => {
        const option: TimeOptions = { maxChars: 0 };
        expect(getTimestamp(1, option)).toBe('000.001');
    });
});

describe('getReadableMem', () => {
    it('when input is empty then return 0kb', () => {
        const input: number[] = [];
        const output = getReadableMem(input);
        expect(output.unit).toBe('KB');
        expect(output.value[0]).toBe(0);
    });
    it('when input is two normal then return max', () => {
        const input: number[] = [];
        input.push(204800); // 204800
        input.push(104800); // 104800
        const output = getReadableMem(input);
        expect(output.unit).toBe('MB');
        expect(output.value[0]).toBe(200);
    });
    it('when input is one normal then return one', () => {
        const input: number[] = [];
        input.push(204800); // 204800
        const output = getReadableMem(input);
        expect(output.unit).toBe('MB');
        expect(output.value[0]).toBe(200);
    });
    it('when input is small one normal then return kb', () => {
        const input: number[] = [];
        input.push(22); // 204800
        const output = getReadableMem(input);
        expect(output.unit).toBe('KB');
        expect(output.value[0]).toBe(22);
    });
});

describe('toLocalTimeString', () => {
    it('when timestamp is normal', () => {
        const input: number = 111111111111111;
        const output = toLocalTimeString(input);
        expect(output).toBe('5490-12-21 13:31:51');
    });
});
