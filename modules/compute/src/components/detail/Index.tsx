/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { Layout } from 'ascend-layout';
import { type Session } from '../../entity/session';
import BaseInfo from './BaseInfo';
import ComputeWorkload from './ComputeWorkload/Index';
import MemoryWorkload from './MemoryWorkload/Index';
import CoreOccupancy from './CoreOccupancy/Index';
import Roofline from './Roofline/Index';

const index = observer(({ session }: { session: Session }): JSX.Element => {
    return (
        <Layout>
            <BaseInfo session={session}/>
            <CoreOccupancy session={session}/>
            <Roofline session={session}/>
            <ComputeWorkload session={session}/>
            <MemoryWorkload session={session}/>
        </Layout>
    );
});

export default index;
