/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
export const loading = (<div style={{ textAlign: 'center', top: '50%', position: 'absolute', width: '50px', left: 'calc(50% - 25px)' }}>
    <div className={'loading'} style={{ marginLeft: '15px' }}></div>
    <div style={{ color: '#dddddd' }}>waiting</div>
</div>);

let logindex: number = 0;
const logRecord: Record<number, unknown> = {};
export function log(...param: unknown[]): void {
    logRecord[logindex++ % 1000] = param;
}
