/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import CollapsiblePanel from 'lib/CollapsiblePanel';
import { type Session } from '../../../entity/session';
import Filter, { defaultCondition, type ICondition } from './Filter';
import CoreChart from './CoreChart';
import { queryCoreOccupancy } from '../../RequestUtils';
import { Advice } from 'lib/CommonUtils';
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
    value: number;
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
    const handleFilterChange = (newCondition: ICondition): void => {
        setCondition({ ...condition, ...newCondition });
    };

    const updateData = async (): Promise<void> => {
        const res = await queryCoreOccupancy();
        const newData = (res ?? defaultData) as ICoreOccupancy;
        setData(newData);
    };

    useEffect(() => {
        if (!session.parseStatus) {
            setData(defaultData);
            return;
        }
        updateData();
    }, [session.parseStatus]);

    return data?.soc?.includes('910B')
        ? (<CollapsiblePanel title={tDetails('Core Occupancy')}>
            <Filter handleFilterChange={handleFilterChange}/>
            <CoreChart condition={condition} session={session} data={data.opDetails}/>
            <Advice text={data.advice}/>
        </CollapsiblePanel>)
        : <></>;
});

export default Index;
