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
export function getTimeOffset(session: Session, cardId?: string): number {
    const unit = session.units.find(value => containCardId(value, cardId ?? ''));
    const realCardId = unit ? (unit.metadata as CardMetaData).cardId : 'Host';
    // 查询泳道chart参数加上时间偏移
    return cardId !== undefined
        ? (session?.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[realCardId] ?? 0
        : 0;
}

// 判断cardUnit自身以及子单元是否包含指定的cardId
function containCardId(unit: InsightUnit, cardId: string): boolean {
    if ((unit.metadata as CardMetaData)?.cardId === cardId) {
        return true;
    }
    return unit.children?.some(childUnit => containCardId(childUnit, cardId)) ?? false;
}
