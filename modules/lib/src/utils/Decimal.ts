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

/**
 * ===== 使用示例 =====
 * "Decimal.add('0.1', '0.2')" => "0.3"
 * "Decimal.sub('1.00', '0.9')" => "0.1"
 * "Decimal.mul('1.23', '4.5')" => "5.535"
 * "Decimal.div('1', '3', 6)" => "0.333333"
 * "Decimal.round('1.005', 2)" => "1.01"
 *
 * 链式调用:
 * "new Decimal('1.005').plus('0.195').times('2').round(3).toString()" => "2.400"
 */

type RoundingMode = 'ROUND_HALF_UP' | 'ROUND_DOWN' | 'ROUND_UP';
const RAW = Symbol('raw');
type RawToken = typeof RAW;

export class Decimal {
    private readonly big: bigint;
    private readonly scale: number;

    private static POW10(n: number): bigint {
        return 10n ** BigInt(n);
    }

    private static toBigIntScale(val: number | string | Decimal): { big: bigint; scale: number } {
        if (val instanceof Decimal) return { big: val.big, scale: val.scale };

        let s = String(val).trim();
        if (s === '') throw new Error('Invalid number: empty');

        if (s.startsWith('.')) s = `0${s}`;
        if (s.startsWith('-.')) s = s.replace('-.', '-0.');

        if (!/^[-+]?\d+(\.\d+)?$/.test(s)) {
            throw new Error(`Invalid number format (scientific notation not supported): ${val}`);
        }

        const neg = s.startsWith('-');
        if (neg || s.startsWith('+')) s = s.slice(1);

        const [i, f = ''] = s.split('.');
        const digits = (i + f).replace(/^0+(?=\d)/, '');
        const big = BigInt((neg ? '-' : '') + (digits || '0'));
        return { big, scale: f.length };
    }

    private static align(a: Decimal, b: Decimal): [Decimal, Decimal] {
        if (a.scale === b.scale) return [a, b];
        if (a.scale > b.scale) {
            return [a, Decimal.fromRaw(b.big * Decimal.POW10(a.scale - b.scale), a.scale)];
        } else {
            return [Decimal.fromRaw(a.big * Decimal.POW10(b.scale - a.scale), b.scale), b];
        }
    }

    constructor(value: number | string | Decimal);
    constructor(big: bigint, scale: number, raw: RawToken);
    constructor(a: number | string | Decimal | bigint, b?: number, c?: RawToken) {
        if (typeof a === 'bigint' && typeof b === 'number' && c === RAW) {
            this.big = a;
            this.scale = b;
        } else {
            const { big, scale } = Decimal.toBigIntScale(a as number | string | Decimal);
            this.big = big;
            this.scale = scale;
        }
    }

    /** 内部工厂方法：直接用 (big, scale) 创建 Decimal 实例，跳过 toBigIntScale 解析 */
    private static fromRaw(big: bigint, scale: number): Decimal {
        return new Decimal(big, scale, RAW);
    }

    // ===== 输出 =====
    toString(): string {
        let s = this.big.toString();
        if (this.scale === 0) return s;
        const neg = s.startsWith('-');
        if (neg) s = s.slice(1);

        if (s.length <= this.scale) s = s.padStart(this.scale + 1, '0');
        const head = s.slice(0, s.length - this.scale);
        const tail = s.slice(s.length - this.scale);
        const body = head + '.' + tail.replace(/0{1,20}$/, '').replace(/\.$/, '');
        return neg ? '-' + (body || '0') : (body || '0');
    }

    // ===== 实例运算 =====
    plus(x: number | string | Decimal): Decimal {
        const B = x instanceof Decimal ? x : new Decimal(x);
        const [A1, B1] = Decimal.align(this, B);
        return Decimal.fromRaw(A1.big + B1.big, A1.scale);
    }

    minus(x: number | string | Decimal): Decimal {
        const B = x instanceof Decimal ? x : new Decimal(x);
        const [A1, B1] = Decimal.align(this, B);
        return Decimal.fromRaw(A1.big - B1.big, A1.scale);
    }

    times(x: number | string | Decimal): Decimal {
        const B = x instanceof Decimal ? x : new Decimal(x);
        return Decimal.fromRaw(this.big * B.big, this.scale + B.scale);
    }

    dividedBy(x: number | string | Decimal, dp = 10, rm: RoundingMode = 'ROUND_HALF_UP'): Decimal {
        const B = x instanceof Decimal ? x : new Decimal(x);
        if (B.big === 0n) throw new Error('Division by zero');

        const scaleAdj = dp + B.scale - this.scale;
        const num = this.big * (scaleAdj >= 0 ? Decimal.POW10(scaleAdj) : 1n);
        const den = B.big * (scaleAdj < 0 ? Decimal.POW10(-scaleAdj) : 1n);

        const q = num / den;
        const r = num % den;
        if (r === 0n) return Decimal.fromRaw(q, dp);

        if (rm === 'ROUND_DOWN') return Decimal.fromRaw(q, dp);

        if (rm === 'ROUND_UP') {
            const bumped = (num >= 0n) === (den >= 0n) ? q + 1n : q - 1n;
            return Decimal.fromRaw(bumped, dp);
        }

        // HALF_UP
        const twiceR = r >= 0n ? 2n * r : -2n * r;
        const absDen = den >= 0n ? den : -den;
        const up = twiceR >= absDen;
        let bumped: bigint;

        if (up) {
            if ((num >= 0n) === (den >= 0n)) {
                bumped = q + 1n;
            } else {
                bumped = q - 1n;
            }
        } else {
            bumped = q;
        }

        return Decimal.fromRaw(bumped, dp);
    }

    round(dp = 0, rm: RoundingMode = 'ROUND_HALF_UP'): Decimal {
        if (dp < 0) throw new Error('dp must be >= 0');
        if (this.scale <= dp) {
            return Decimal.fromRaw(this.big * Decimal.POW10(dp - this.scale), dp);
        }
        const cut = this.scale - dp;
        const p10 = Decimal.POW10(cut);
        const base = this.big / p10;
        const rem = this.big % p10;

        if (rm === 'ROUND_DOWN') return Decimal.fromRaw(base, dp);
        if (rm === 'ROUND_UP') {
            if (rem === 0n) return Decimal.fromRaw(base, dp);
            return Decimal.fromRaw(this.big >= 0n ? base + 1n : base - 1n, dp);
        }
        // HALF_UP
        const half = p10 / 2n;
        const up = this.big >= 0n ? rem >= half : -rem >= half;
        let raw: bigint;

        if (up) {
            if (this.big >= 0n) {
                raw = base + 1n;
            } else {
                raw = base - 1n;
            }
        } else {
            raw = base;
        }

        return Decimal.fromRaw(raw, dp);
    }

    // ===== 静态方法，方便直接调用 =====
    static add(a: number | string, b: number | string): string {
        return new Decimal(a).plus(b).toString();
    }

    static sub(a: number | string, b: number | string): string {
        return new Decimal(a).minus(b).toString();
    }

    static mul(a: number | string, b: number | string): string {
        return new Decimal(a).times(b).toString();
    }

    static div(a: number | string, b: number | string, dp = 10, rm: RoundingMode = 'ROUND_HALF_UP'): string {
        return new Decimal(a).dividedBy(b, dp, rm).toString();
    }

    static round(a: number | string, dp = 0, rm: RoundingMode = 'ROUND_HALF_UP'): string {
        return new Decimal(a).round(dp, rm).toString();
    }
}
