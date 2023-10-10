/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { Container, formatDate, Loading } from '../Common';
import { StringMap } from '../../utils/interface';
import { Session } from '../../entity/session';
import { queryTopSummary } from '../../utils/RequestUtils';
import { defaultConditions } from './Filter';

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

const initBaseInfo = async (setData: any): Promise<void> => {
    const res: any = await queryTopSummary(defaultConditions);
    setData({
        ...res,
        collectDuration: formateTime(Number(res.collectDuration)),
        collectStartTime: formatDate(new Date(res.collectStartTime)),
        dataSize: res.dataSize !== undefined && res.dataSize > 0.01 ? Number(res.dataSize?.toFixed(2)) : res.dataSize,
    });
};

const getDisplayItems = (session: Session): any[] => {
    if (session.unitcount === 0) {
        return list.filter(item => ![ 'collectStartTime', 'collectDuration' ].includes(item.key));
    }
    return list;
};
const BaseInfo = ({ session }: { session: Session}): JSX.Element => {
    const [ data, setData ] = useState<StringMap>({});
    useEffect(() => {
        setTimeout(() => {
            initBaseInfo(setData);
        });
    }, [ session.parseCompleted, session.renderId ]);
    const displaylist = getDisplayItems(session);
    return <Container
        title={'Base Info'}
        titleClassName={'common-title-bottom'}
        style={{ height: 'auto' }}
        content={(<div className={'baseinfo'}>
            {
                displaylist.map(item => (
                    <div key={item.key}>
                        <div>{item.label}:</div>
                        <div>
                            {
                                item.key === 'collectDuration' && !session.parseCompleted
                                    ? <Loading style={{ marginTop: '10px' }}/>
                                    : data[item.key]
                            }
                        </div>
                    </div>
                ))
            }
        </div>)}
    />;
};

export default BaseInfo;
