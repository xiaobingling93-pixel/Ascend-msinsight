/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { Session } from '@/entity/session';
import TabPane from './TabPane/Index';
import ToolBox from './ToolBox/Index';

interface IProps {
    session: Session;
}
const Main = observer(({ session }: IProps) => {
    return <div style={{ height: '100%' }}>
        <ToolBox />
        <TabPane session={session}/>
    </div>;
});

export default Main;
