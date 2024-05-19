/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { type Session } from '../../entity/session';
import BaseInfo from './BaseInfo';
import ComputeWorkload from './ComputeWorkload/Index';
import MemoryWorkload from './MemoryWorkload/Index';

const index = observer(({ session }: { session: Session }): JSX.Element => {
    return (
        <div style={{ padding: '0 20px', height: '100%', overflow: 'auto' }}>
            <div>
                <BaseInfo session={session}/>
                <ComputeWorkload session={session}/>
                <MemoryWorkload session={session}/>
            </div>
        </div>
    );
});

export default index;
