/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { Session } from '@/entity/session';

interface IProps {
    session: Session;
}
const Index = observer(({ session }: IProps) => {
    return <div>Remote Manger</div>;
});

export default Index;
