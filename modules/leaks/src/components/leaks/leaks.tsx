/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { Session } from '../../entity/session';
import { Layout } from '@insight/lib/components';
import MemoryStack from '../MemoryStack';

const index = observer((props: { session: Session }) => {
    const { session } = props;
    return (<Layout> <MemoryStack session={session} /> </Layout>);
});

export default index;
