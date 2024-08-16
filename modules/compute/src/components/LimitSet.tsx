/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import i18n from 'ascend-i18n';

export function LimitHit({ maxSize = 0, name = '' }: {maxSize?: number;name?: string}): JSX.Element {
    return (<div style={{ color: 'red' }}>{i18n.t('LimitWarn', { maxSize, name })}</div>);
}
