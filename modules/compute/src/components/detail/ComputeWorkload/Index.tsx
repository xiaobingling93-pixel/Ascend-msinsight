/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import ComputeWorkloadChart from './ComputeWorkloadChart';
import ComputeWorkloadTable from './ComputeWorkloadTable';
import { type Session } from '../../../entity/session';
import { Hit, sortFunc } from '@insight/lib/utils';
import Filter, { defaultCondition, type Icondition } from './Filter';
import { queryComputeWorkload } from '../../RequestUtils';
import { CollapsiblePanel } from '@insight/lib/components';
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

    const getBaseInfo = async (isCompared: boolean): Promise<void> => {
        const res = await queryComputeWorkload({ isCompared });
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
            setData(defaultData);
            return;
        }
        getBaseInfo(session.dirInfo.isCompare);
    }, [session.updateId, session.parseStatus, session.dirInfo]);
    useEffect(() => {
        runInAction(() => {
            session.blockIdList = data.blockIdList;
        });
    }, [data.blockIdList]);
    return (
        <CollapsiblePanel title={t('ComputeWorkloadAnalysis')} collapsible>
            <Filter handleFilterChange={handleFilterChange} blockIdList={data.blockIdList} session={session}/>
            <ComputeWorkloadChart condition={condition} data={data.chartData}/>
            { Number(session?.computeAdvice?.length) > 0 && (<Hit text={session.computeAdvice} style={{ marginBottom: '10px' }} type={'alarm'} data-testId={'computeWorkloadAdivce'}/>) }
            <ComputeWorkloadTable condition={condition} data={data.tableData}/>
        </CollapsiblePanel>
    );
});

export default index;
