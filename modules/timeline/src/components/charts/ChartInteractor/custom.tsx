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
import * as React from 'react';
import type { Theme } from '@emotion/react';
import type { Session } from '../../../entity/session';
import type { Scale } from '../../../entity/chart';

// This is intentionally made a static global variable
const customRenderEffects: CustomRenderEffect[] = [];

export type CustomCrossRenderer = (ctx: CanvasRenderingContext2D | null, session: Session, xScale: Scale, theme: Theme) => void;

export interface CustomRenderEffect {
    action: CustomCrossRenderer;
    triggers: (session: Session) => unknown[];
};

export const registerCrossUnitRenderer = (effect: CustomRenderEffect): void => {
    customRenderEffects.push(effect);
};

export const useCustomRenderers = (session: Session): [ CustomCrossRenderer[], unknown[] ] => {
    const customRenderers = React.useMemo(() => customRenderEffects.map(it => it.action), [session]);
    const customRenderTriggers = customRenderEffects.flatMap(it => it.triggers(session));
    return [customRenderers, customRenderTriggers];
};
