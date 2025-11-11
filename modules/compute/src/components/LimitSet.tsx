/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { CSSProperties } from 'react';
import i18n from '@insight/lib/i18n';

export function LimitHit({ maxSize = 0, name = '', style = {} }: {maxSize?: number;name?: string;style?: CSSProperties}): JSX.Element {
    return (<div style={{ ...style, color: 'red' }}>{i18n.t('LimitWarn', { maxSize, name })}</div>);
}
