/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useEffect, useState } from 'react';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { DynamicTooltip } from '@insight/lib/components';
import { Session } from '../../../entity/session';
import { store } from '../../../store';

export const showFormula = (params: {event: MouseEvent;name?: string;id?: string}): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        const pos = (params.event.target as HTMLElement)?.getBoundingClientRect();
        session.memoryIndicator = { x: pos.x ?? params.event.pageX, y: pos.y ?? params.event.pageY, name: params.name ?? params.id ?? '' };
    });
};

export const hideFormula = (): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.memoryIndicator = { x: 0, y: 0 };
    });
};

const formulaMap: Record<string, string> = {
    hitRatio: 'L2 Cache Hit Rate Formula',
    cubeRatio: 'Cube Ratio Formula',
    vectorRatio: 'Vector Ratio Formula',
};

interface TipConfig {
    x: number;
    y: number;
    content: null | React.ReactNode[];
}

// 公式提示框
export const FormulaTip = observer(({ session }: { session: Session}) => {
    const { t } = useTranslation('details');
    const [tipConfig, setTipConfig] = useState<TipConfig>({ x: 0, y: 0, content: null });

    useEffect(() => {
        const { x, y, name = '' } = session.memoryIndicator;
        const formula = formulaMap[name];
        setTipConfig({ x, y, content: formula !== undefined ? [t(formula)] : null });
    }, [session.memoryIndicator]);

    return <DynamicTooltip
        mouseX={tipConfig.x}
        mouseY={tipConfig.y}
        content={tipConfig.content}
        position="fixed"
        placement={'topLeft'}
        animation={false}
    />;
});
