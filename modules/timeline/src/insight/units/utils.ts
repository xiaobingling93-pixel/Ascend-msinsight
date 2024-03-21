import type { Theme } from '@emotion/react';
import type { Session } from '../../entity/session';
import type { CardMetaData } from '../../entity/data';

export const colorPalette: Array<keyof Theme['colorPalette']> = ['slateblue', 'royalblue', 'skyblue', 'turquoise', 'olivedrab', 'yellowgreen',
    'gold', 'orange', 'coral', 'orangered', 'palevioletred', 'mediumorchid'];

export function getTimeOffset(session: Session, cardId?: string): number {
    const unit = session.units.find(value => (value.metadata as CardMetaData).cardId === cardId);
    const realCardId = unit ? (unit.metadata as CardMetaData).cardId : 'Host';
    // 查询泳道chart参数加上时间偏移
    return cardId !== undefined
        ? (session?.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[realCardId] ?? 0
        : 0;
}
