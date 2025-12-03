/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import type { Theme } from '@emotion/react';
import type { Session } from '../../entity/session';
import type { CardMetaData } from '../../entity/data';
import type { InsightUnit } from '../../entity/insight';

export const colorPalette: Array<keyof Theme['colorPalette']> = [
    'deepBlue',
    'coralRed',
    'tealGreen',
    'aquaBlue',
    'raspberryPink',
    'vividBlue',
    'vividRed',
    'royalPurple',
    'skyBlue',
    'sunsetOrange',
    'amethystPurple',
    'limeGreen',
];
export function getTimeOffset(session: Session, metaData: { cardId?: string; processId?: string }, units: InsightUnit[] = [], timestampOffset?: Record<string, number>): number {
    const timeOffsetKey = getTimeOffsetKey(session, metaData, units);
    timestampOffset = timestampOffset ?? session?.unitsConfig.offsetConfig.timestampOffset;
    // 查询泳道chart参数加上时间偏移
    return metaData.cardId !== undefined
        ? timestampOffset?.[timeOffsetKey] ?? 0
        : 0;
}

export function getTimeOffsetKey(session: Session, metaData: { cardId?: string; processId?: string }, units: InsightUnit[] = []): string {
    if (units.length === 0) {
        units = session.units;
    }
    const unit = units.find(value => containCardId(value, metaData.cardId ?? ''));
    const realCardId = unit ? (unit.metadata as CardMetaData).cardId : 'Host';
    let realProcessId = metaData.processId;
    // db数据的Host侧有2层process类型的泳道，第二层的processId的前32位是第一层的ProcessId，后32位是本泳道的threadId
    if (realProcessId !== undefined && !isNaN(Number(realProcessId))) {
        const upper32BitProcessId = Math.floor(Number(realProcessId) / Math.pow(2, 32));
        if (upper32BitProcessId !== 0) {
            realProcessId = upper32BitProcessId.toString();
        }
    }
    return (realProcessId != null) ? `${realCardId}__${realProcessId}` : realCardId;
}

// 判断cardUnit自身以及子单元是否包含指定的cardId
function containCardId(unit: InsightUnit, cardId: string): boolean {
    if ((unit.metadata as CardMetaData)?.cardId === cardId) {
        return true;
    }
    return unit.children?.some(childUnit => containCardId(childUnit, cardId)) ?? false;
}

function parseDecimal(str: string): {
    sign: bigint;
    int: string;
    frac: string;
} {
    let sign = BigInt(1);
    if (str.startsWith('-')) {
        sign = BigInt(-1);
        str = str.slice(1);
    } else if (str.startsWith('+')) {
        str = str.slice(1);
    }
    const [intPart, fracPart = ''] = str.split('.');
    return { sign, int: intPart || '0', frac: fracPart };
}

export function bigSubtract(a: number | string, b: number | string): string {
    if (typeof BigInt === 'undefined') {
        return '-';
    }
    // 转为字符串，避免 number 精度丢失
    const aStr = String(a);
    const bStr = String(b);

    // 拆分整数/小数部分
    const aParser = parseDecimal(aStr);
    const bParser = parseDecimal(bStr);

    // 统一小数长度（取较长的小数位数）
    const maxFracLen = Math.max(aParser.frac.length, bParser.frac.length);
    const aFracPadded = aParser.frac.padEnd(maxFracLen, '0');
    const bFracPadded = bParser.frac.padEnd(maxFracLen, '0');

    // 拼接整数+小数 → 大整数
    const aBig = aParser.sign * BigInt(aParser.int + aFracPadded);
    const bBig = bParser.sign * BigInt(bParser.int + bFracPadded);

    // 执行减法
    let resultBig = aBig - bBig;

    // 结果符号
    let signStr = '';
    if (resultBig < 0) {
        signStr = '-';
        resultBig = -resultBig;
    }

    // 转字符串并插入小数点
    let resultStr = resultBig.toString().padStart(maxFracLen + 1, '0');
    if (maxFracLen > 0) {
        const intPart = resultStr.slice(0, -maxFracLen);
        const fracPart = resultStr.slice(-maxFracLen);
        resultStr = intPart + '.' + fracPart;
    }

    // 去掉前导零 & 末尾多余的零
    resultStr = resultStr.replace(/^0+(?=\d)/, ''); // 去掉多余前导零
    if (resultStr.includes('.')) {
        // 先去掉小数点后多余的 0
        resultStr = resultStr.replace(/(\.\d*?[1-9])0+$/, '$1');
        // 再去掉类似 "123." 的末尾小数点
        resultStr = resultStr.replace(/\.$/, '');
    }

    return signStr + resultStr;
}
