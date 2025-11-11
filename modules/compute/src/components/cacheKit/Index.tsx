/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { type Session } from '../../entity/session';
import TraceKitChart from './CacheKitChart';
import { Layout } from '@insight/lib';

const index = observer(({ session }: { session: Session }): JSX.Element => {
    return (
        <Layout>
            <TraceKitChart session={session}/>
        </Layout>
    );
});

export default index;
