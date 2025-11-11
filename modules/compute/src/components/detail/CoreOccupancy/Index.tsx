/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { CollapsiblePanel } from '@insight/lib/components';
import { type Session } from '../../../entity/session';
import Filter, { defaultCondition, type ICondition } from './Filter';
import CoreChart from './CoreChart';
import { queryCoreOccupancy } from '../../RequestUtils';
import { Hit } from '@insight/lib/utils';
import { CompareData } from '../../../utils/interface';
export interface ICoreOccupancy {
    soc: string; // 算子运行平台
    opType: string; // 算子类型：vector, cube, mix
    advice: string; // 专家建议
    opDetails: ICore[] ;
}

export interface ICore {
    coreId: number;
    subCoreDetails: ISubCore[];
}
export interface ISubCore {
    subCoreName: string;
    cycles: IData;
    throughput: IData;
    cacheHitRate: IData;
}

export interface IData {
    value: CompareData<number>;
    level: number;
}

const defaultData = {
    soc: '',
    opType: '',
    advice: '',
    opDetails: [],
};

const Index = observer(({ session }: { session: Session }): JSX.Element => {
    const [condition, setCondition] = useState<ICondition>(defaultCondition);
    const [data, setData] = useState<ICoreOccupancy>(defaultData);
    const { t: tDetails } = useTranslation('details');
    const hasInfo = data?.opDetails?.length > 0 || (data?.soc !== undefined && data?.soc !== null && data?.soc !== '');

    const handleFilterChange = (newCondition: ICondition): void => {
        setCondition({ ...condition, ...newCondition });
    };

    const updateData = async (isCompared: boolean): Promise<void> => {
        try {
            const res = await queryCoreOccupancy(isCompared);
            const newData = (res ?? defaultData) as ICoreOccupancy;
            setData(newData);
        } catch (expected) {
            // 接口异常
            setData(defaultData);
        }
    };

    useEffect(() => {
        if (!session.parseStatus) {
            setData(defaultData);
            return;
        }
        updateData(session.dirInfo.isCompare);
    }, [session.parseStatus, session.dirInfo]);

    return hasInfo
        ? (<CollapsiblePanel title={tDetails('Core Occupancy')} collapsible id={'coreOccupancy'}>
            <Filter handleFilterChange={handleFilterChange} session={session}/>
            <CoreChart condition={condition} session={session} data={data}/>
            {data?.advice?.length > 0 && (<Hit text={data.advice} style={{ marginTop: '10px' }} type={'alarm'}/>)}
        </CollapsiblePanel>)
        : <></>;
});

export default Index;
