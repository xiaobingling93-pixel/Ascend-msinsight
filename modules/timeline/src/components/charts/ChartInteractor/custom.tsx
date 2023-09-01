import * as React from 'react';
import type { Theme } from '@emotion/react';
import { Session } from '../../../entity/session';
import { Scale } from '../../../entity/chart';

// This is intentionally made a static global variable
const customRenderEffects: CustomRenderEffect[] = [];

export type CustomCrossRenderer = (ctx: CanvasRenderingContext2D | null, session: Session, xScale: Scale, theme: Theme) => void;

export type CustomRenderEffect = {
    action: CustomCrossRenderer;
    triggers: (session: Session) => unknown[];
};

export const registerCrossUnitRenderer = (effect: CustomRenderEffect): void => {
    customRenderEffects.push(effect);
};

export const useCustomRenderers = (session: Session): [ CustomCrossRenderer[], unknown[] ] => {
    const customRenderers = React.useMemo(() => customRenderEffects.map(it => it.action), [session]);
    const customRenderTriggers = customRenderEffects.flatMap(it => it.triggers(session));
    return [ customRenderers, customRenderTriggers ];
};
