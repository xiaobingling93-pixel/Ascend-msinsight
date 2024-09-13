/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { type Session } from '../../entity/session';
import TraceKitChart from './CacheKitChart';
import { Layout } from 'ascend-layout';

const index = observer(({ session }: { session: Session }): JSX.Element => {
    return (
        <Layout>
            <TraceKitChart session={session}/>
        </Layout>
    );
});

export default index;
