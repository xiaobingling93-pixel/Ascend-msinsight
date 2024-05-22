/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import type { Session } from '../../entity/session';
import './Jupyter.css';
import { loading } from '../Common';

const index = observer((props: { session: Session }) => {
    const { session } = props;
    if (session.isIpynb && session.ipynbUrl !== '') {
        return <iframe className='jupyter-iframe' src={session.ipynbUrl}></iframe>;
    } else {
        return loading;
    }
});

export default index;
