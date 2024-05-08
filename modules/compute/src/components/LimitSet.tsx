/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';

const MAX_SIZE = 10000; // 1万

export function LimitHit({ maxSize, name }: {maxSize?: number;name?: string}): JSX.Element {
    return (<div style={{ color: 'red' }}>{`Warn : ${name ?? ''} exceed the limit , Only display the first ${maxSize ?? MAX_SIZE} .`}</div>);
}
