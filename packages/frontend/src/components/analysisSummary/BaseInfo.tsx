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

const formateTime = (t: number): string => {
    if (isNaN(t)) {
        return '';
    }
    let leftTime = t;
    // 小时级
    if (t >= 1000 * 1000 * 60 * 60) {
        const h = Math.floor(leftTime / (1000 * 1000 * 60 * 60));
        leftTime = leftTime % (1000 * 1000 * 60 * 60);
        const m = Math.floor(leftTime / (1000 * 1000 * 60));
        leftTime = leftTime % (1000 * 1000 * 60);
        const s = Number((leftTime / (1000 * 1000)).toFixed(2));
        return `${h}h${m}m${s}s`;
    }
    // 分钟级
    if (t >= 1000 * 1000 * 60) {
        const m = Math.floor(leftTime / (1000 * 1000 * 60));
        leftTime = leftTime % (1000 * 1000 * 60);
        const s = Number((leftTime / (1000 * 1000)).toFixed(2));
        return `${m}m${s}s`;
    }
    // 秒级
    if (t >= 1000 * 1000) {
        const s = Number((leftTime / (1000 * 1000)).toFixed(2));
        return `${s}s`;
    }
    // 毫秒级
    if (t >= 1000) {
        const s = Number((leftTime / (1000)).toFixed(2));
        return `${s}ms`;
    }
    return `${t}μs`;
};

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
                        <div>{
                            item.key === 'collectDuration'
                                ? formateTime(Number(data.collectDuration))
                                : data[item.key]
                        }</div>
                    </div>
                ))
            }
        </div>)}
    />;
};

export default BaseInfo;
