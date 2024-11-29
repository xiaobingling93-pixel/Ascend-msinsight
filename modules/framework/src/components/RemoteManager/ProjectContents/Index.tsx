/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { store } from '@/store';
import Contents from './Contents';

const Index = observer(() => {
    const session = store.sessionStore.activeSession;
    if (!session) {
        return <></>;
    }

    return <Contents session={session}/>;
});

export default Index;
