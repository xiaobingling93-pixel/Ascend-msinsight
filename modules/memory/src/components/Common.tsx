/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React from 'react';
import { Tooltip } from 'antd';
import { QuestionCircleFilled } from '@ant-design/icons';

export const Label = (props: {name: React.ReactNode;style?: object }): JSX.Element => {
    return <span style={{ margin: '0 10px', ...(props.style ?? {}) }}>{props.name}{' :'} </span>;
};

export const hit = (<Tooltip color={'#2e2f31'} title={
    (
        <div style={{ background: '#1e1e1e', padding: '1rem' }}>
            <div>Overall: Displays the memory sizes of operators in the Reserved, Allocated,
                and Active states and the overall Memory Reserved size of PyTorch.</div>
            <div style={{ marginTop: '2rem' }}>Stream: Displays the memory sizes of operators
                in the Reserved, Allocated, and Active states by stream.</div>
        </div>
    )
}>
    <QuestionCircleFilled style={{ cursor: 'pointer', margin: '0 0 0 10px' }}/>
</Tooltip>);
