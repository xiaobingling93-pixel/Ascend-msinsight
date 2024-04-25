/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import BaseInfo from './BaseInfo';
import { type Session } from '../../entity/session';

function Index({ session }: { session: Session }): JSX.Element {
    return (
        <div style={{ padding: '0 20px' }}>
            <BaseInfo session={session}/>
        </div>
    );
}

export default Index;
