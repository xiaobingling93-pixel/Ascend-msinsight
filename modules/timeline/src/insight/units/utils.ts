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
export function getTimeOffset(session: Session, metaData: { cardId?: string; processId?: string }): number {
    const timeOffsetKey = getTimeOffsetKey(session, metaData);
    // 查询泳道chart参数加上时间偏移
    return metaData.cardId !== undefined
        ? session?.unitsConfig.offsetConfig.timestampOffset?.[timeOffsetKey] ?? 0
        : 0;
}

export function getTimeOffsetKey(session: Session, metaData: { cardId?: string; processId?: string }): string {
    const unit = session.units.find(value => containCardId(value, metaData.cardId ?? ''));
    const realCardId = unit ? (unit.metadata as CardMetaData).cardId : 'Host';
    let realProcessId = metaData.processId;
    // db数据的Host侧有2层process类型的泳道，第二层的processId的前32位是第一层的ProcessId，后32位是本泳道的threadId
    if (realCardId.endsWith('Host') && realProcessId !== undefined && !isNaN(Number(realProcessId))) {
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
