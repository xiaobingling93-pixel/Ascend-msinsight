/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React from 'react';
import { Container } from '../Common';
import { StringMap } from '../../utils/interface';
import { Session } from '../../entity/session';

export interface BaseInfoDataType{
    collectDuration: string | number | undefined;
    [prop: string]: any;
}

export const defaultBaseInfo = {
    filePath: '',
    dataSize: '',
    collectStartTime: '',
    rankCount: '',
    stepNum: '',
    collectDuration: '',
};

const list = [
    {
        label: 'Report file',
        key: 'filePath',
        value: '',
    },
    {
        label: 'Report size(MB)',
        key: 'dataSize',
    },
    {
        label: 'Report capture time',
        key: 'collectStartTime',
    },
    {
        label: 'Device count',
        key: 'rankCount',
    },
    {
        label: 'Step count',
        key: 'stepNum',
    },
    {
        label: 'Profiling session duration',
        key: 'collectDuration',
    },
];

const BaseInfo = ({ data }: {data: StringMap;session: Session}): JSX.Element => {
    return <Container
        title={'BaseInfo'}
        titleClassName={'common-title-bottom'}
        style={{ height: 'auto' }}
        content={(<div className={'baseinfo'}>
            {
                list.map(item => (
                    <div key={item.key}>
                        <div>{item.label}:</div>
                        <div>{data[item.key]}</div>
                    </div>
                ))
            }
        </div>)}
    />;
};

export default BaseInfo;
