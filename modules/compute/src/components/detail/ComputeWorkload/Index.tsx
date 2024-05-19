/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import ComputeWorkloadChart from './ComputeWorkloadChart';
import ComputeWorkloadTable from './ComputeWorkloadTable';
import { type Session } from '../../../entity/session';
import { BaseContainer, sortFunc } from 'lib/CommonUtils';
import Filter, { defaultCondition, type Icondition } from './Filter';
import { queryComputeWorkload } from '../../RequestUtils';

export interface IblockData {
    blockId: string;
    blockType: string;
    name: string;
    unit: string;
    value: string;
    originValue: string;
}
interface Idata {
    blockIdList: string[];
    chartData: IblockData[];
    tableData: IblockData[];
}
const defaultData = {
    blockIdList: [],
    chartData: [],
    tableData: [],
};

const index = observer(({ session }: { session: Session }): JSX.Element => {
    const [data, setData] = useState<Idata>(defaultData);
    const [condition, setCondition] = useState(defaultCondition);

    const getBaseInfo = async (): Promise<void> => {
        const res = await queryComputeWorkload();
        if (res === null || res === undefined) {
            return;
        }
        const renderData = {
            blockIdList: res.blockIdList ?? [],
            chartData: res.chartData?.detailDataList ?? [],
            tableData: res.tableData?.detailDataList ?? [],
        } as Idata;
        renderData.blockIdList.sort((a, b) => sortFunc(a, b, 'asc'));
        setData(renderData);
    };
    const handleFilterChange = (newCondition: Icondition): void => {
        setCondition({ ...condition, ...newCondition });
    };
    useEffect(() => {
        getBaseInfo();
    }, [session.updateId]);
    useEffect(() => {
        runInAction(() => {
            session.blockIdList = data.blockIdList;
        });
    }, [data.blockIdList]);
    return (
        <BaseContainer
            header="Compute Workload Analysis"
            body={<div style={{ padding: '10px 20px' }}>
                <Filter handleFilterChange={handleFilterChange} blockIdList={data.blockIdList}/>
                <ComputeWorkloadChart blockId={condition.blockId} data={data.chartData}/>
                <ComputeWorkloadTable blockId={condition.blockId} data={data.tableData}/>
            </div>}
        />
    );
});

export default index;
