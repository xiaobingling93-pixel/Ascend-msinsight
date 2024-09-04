/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import ComputeWorkloadChart from './ComputeWorkloadChart';
import ComputeWorkloadTable from './ComputeWorkloadTable';
import { type Session } from '../../../entity/session';
import { sortFunc } from 'ascend-utils';
import Filter, { defaultCondition, type Icondition } from './Filter';
import { queryComputeWorkload } from '../../RequestUtils';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { CompareData } from '../../../utils/interface';

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
    chartData: Array<CompareData<IblockData>>;
    tableData: Array<CompareData<IblockData>>;
}
const defaultData = {
    blockIdList: [],
    chartData: [],
    tableData: [],
};

const index = observer(({ session }: { session: Session }): JSX.Element => {
    const [data, setData] = useState<Idata>(defaultData);
    const [condition, setCondition] = useState(defaultCondition);
    const { t } = useTranslation('details');

    const getBaseInfo = async (): Promise<void> => {
        const res = await queryComputeWorkload();
        const renderData = {
            blockIdList: res?.blockIdList ?? [],
            chartData: res?.chartData?.detailDataList ?? [],
            tableData: res?.tableData?.detailDataList ?? [],
        } as Idata;
        renderData.blockIdList.sort((a, b) => sortFunc(a, b, 'asc'));
        setData(renderData);
    };
    const handleFilterChange = (newCondition: Icondition): void => {
        setCondition({ ...condition, ...newCondition });
    };
    useEffect(() => {
        if (!session.parseStatus) {
            setTimeout(() => {
                if (!session.parseStatus) {
                    setData(defaultData);
                }
            }, 200);
            return;
        }
        getBaseInfo();
    }, [session.updateId, session.parseStatus]);
    useEffect(() => {
        runInAction(() => {
            session.blockIdList = data.blockIdList;
        });
    }, [data.blockIdList]);
    return (
        <CollapsiblePanel title={t('ComputeWorkloadAnalysis')}>
            <Filter handleFilterChange={handleFilterChange} blockIdList={data.blockIdList}/>
            <ComputeWorkloadChart blockId={condition.blockId} data={data.chartData}/>
            <ComputeWorkloadTable blockId={condition.blockId} data={data.tableData}/>
        </CollapsiblePanel>
    );
});

export default index;
