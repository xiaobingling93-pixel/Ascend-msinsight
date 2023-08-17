/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import * as echarts from 'echarts';
import React, { useEffect, useState } from 'react';
import Filter, { ConditionDataType } from './Filter';
import StatisticsTable from './StatisticsTable';
import { VoidFunction } from '../../utils/interface';
import SummaryTable from './SummaryTable';
import { queryTopSummary } from '../../utils/RequestUtils';
import BaseInfo, { BaseInfoDataType, defaultBaseInfo } from './BaseInfo';
import { formatDate, isNull } from '../Common';
import { Tooltip } from 'antd';
import { QuestionCircleFilled, ExclamationCircleFilled } from '@ant-design/icons';
import { Session } from '../../entity/session';

interface SummaryDataType{
    rankId: string ;
    computingTime: number;
    communicationNotOverLappedTime: number;
    communicationOverLappedTime: number;
    freeTime: number;
    ComputeTimeRatio?: string | number;
    CommunicationTimeRatio?: string | number;
    [propName: string]: any;
}

const baseOption: any = {
    tooltip: {
        trigger: 'axis',
        axisPointer: {
            type: 'shadow',
        },
    },
    legend: {
        data: [
            { name: 'Computing', textStyle: { color: 'rgb(123,122,122)' } },
            { name: 'Communication(Not Overlapped)', textStyle: { color: 'rgb(123,122,122)' } },
            { name: 'Communication(Overlapped)', textStyle: { color: 'rgb(123,122,122)' } },
            { name: 'Free', textStyle: { color: 'rgb(123,122,122)' } },
            { name: 'Computing Ratio', textStyle: { color: 'rgb(123,122,122)' } },
            { name: 'Communication Ratio', textStyle: { color: 'rgb(123,122,122)' } },
        ],
    },
    grid: {
        left: '3%',
        right: '4%',
        bottom: '3%',
        containLabel: true,
    },
    xAxis: [
        {
            type: 'category',
            data: [ ],
            name: 'Rank',
            nameLocation: 'start',
            nameGap: 15,
            nameTextStyle: { fontWeight: 'bold', fontSize: '1rem', padding: [ 0, 10, 0, 0 ] },
        },
    ],
    yAxis: [
        {
            type: 'value',
            name: 'Time(ms)',
            axisLabel: {
                formatter: '{value}',
            },
        },
        {
            type: 'value',
            name: 'Ratio',
            min: 0,
            axisLabel: {
                formatter: '{value}%',
            },
        },
    ],
    series: [
        {
            id: 'compute',
            name: 'Computing',
            type: 'bar',
            stack: 'Ad',
            emphasis: {
                focus: 'series',
            },
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + ' ms';
                },
            },
            data: [ ],
        },
        {
            id: 'communicationNotOverLappedTime',
            name: 'Communication(Not Overlapped)',
            type: 'bar',
            stack: 'Ad',
            emphasis: {
                focus: 'series',
            },
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + ' ms';
                },
            },
            data: [ ],
        },
        {
            id: 'communicationOverLappedTime',
            name: 'Communication(Overlapped)',
            type: 'bar',
            stack: 'Ad',
            emphasis: {
                focus: 'series',
            },
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + ' ms';
                },
            },
            data: [ ],
        },
        {
            id: 'freeTime',
            name: 'Free',
            type: 'bar',
            stack: 'Ad',
            emphasis: {
                focus: 'series',
            },
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + ' ms';
                },
            },
            data: [ ],
        },
        {
            name: 'Computing Ratio',
            type: 'line',
            yAxisIndex: 1,
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + ' %';
                },
            },
            data: [ ],
        },
        {
            name: 'Communication Ratio',
            type: 'line',
            yAxisIndex: 1,
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + ' %';
                },
            },
            data: [ ],
        },
    ],
};
function wrapData(data: SummaryDataType[]): any {
    const list = [ 'computingTime', 'communicationNotOverLappedTime', 'freeTime' ];
    data.forEach(item => {
        let total = 0;
        list.forEach(field => {
            item[field] = Number(item[field].toFixed(2));
            total += item[field];
        });
        item.computeTimeRatio = (100 * item.computingTime / total).toFixed(2);
        item.communicationTimeRatio = (100 * item.communicationNotOverLappedTime / total).toFixed(2);
    });
    baseOption.xAxis[0].data = data.map(item => item.rankId);
    const order: Array<keyof SummaryDataType> = [ 'computingTime', 'communicationNotOverLappedTime',
        'communicationOverLappedTime', 'freeTime', 'computeTimeRatio', 'communicationTimeRatio' ];
    for (let i = 0; i < order.length; i++) {
        baseOption.series[i].data = data.map(item => item[order[i]]);
    }
    return baseOption;
}

async function initCharts(data: any, handleClick: VoidFunction): Promise<void> {
    const chartDom = document.getElementById('overview-chart');
    if (chartDom !== null) {
        echarts.init(chartDom).dispose();
        const myChart = echarts.init(chartDom);
        myChart.setOption(wrapData(data));
        myChart.on('click', handleClick);
    }
}
export const hit = (<Tooltip title={
    (
        <div style={{ background: 'var(--grey100)', padding: '1rem' }}>
            <div>总时间 = Computing + Communication(Not Overlapped) + Free</div>
            <div>Computing Ratio = ( Computing + Communication(Not Overlapped) + Free ) / 总时间</div>
            <div>Communication Ratio = Communication(Not Overlapped) / 总时间</div>
            <div style={{ marginTop: '2rem' }}><ExclamationCircleFilled />
                点击不同卡的柱状图展示单卡Computation / Communication详情</div>
        </div>
    )
}>
    <QuestionCircleFilled style={{ cursor: 'pointer', margin: '0 10px' }}/>
</Tooltip>);

const ComputationCommunicationOverview = ({ session }: { session: Session }): JSX.Element => {
    const [ groupData, setGroupData ] = useState({ rankList: [], stepList: [], init: false });
    const [ dataSource, setDatasource ] = useState<SummaryDataType[]>([]);
    const [ allDataSource, setAllDatasource ] = useState<SummaryDataType[]>([]);
    const [ selected, setSelected ] = useState({ rankId: '', timeFlag: '' });
    const [ baseInfo, setBaseInfo ] = useState<BaseInfoDataType>(defaultBaseInfo);
    useEffect(() => {
        handleFilterChange({ step: 'All', rankIds: [], orderBy: 'computingTime', top: 0 });
    }, [ ]);
    useEffect(() => {
        initCharts(dataSource, handleClick);
    }, [dataSource]);
    useEffect(() => {
        if (isNull(baseInfo.collectDuration)) {
            setBaseInfo({ ...baseInfo, collectDuration: session.endTimeAll });
        }
    }, [session.endTimeAll]);
    const handleFilterChange = async (conditions: ConditionDataType, doQuery?: boolean): Promise<void> => {
        if (doQuery === false) {
            let data = [...allDataSource];
            data = data.slice(0, conditions.top);
            setDatasource(data);
            return;
        }
        const res: any = await queryTopSummary(conditions);
        const { summaryList, rankList, stepList } = res.result;
        setDatasource(summaryList);
        const data = summaryList.slice(0, conditions.top);
        setAllDatasource(data);
        if (!groupData.init) {
            setGroupData({ rankList, stepList, init: true });
            setBaseInfo({ ...res.result, collectStartTime: formatDate(new Date(res.result.collectStartTime)) });
            setSelected({ ...selected, rankId: rankList[0] });
        }
    };

    const handleClick = (param: any): void => {
        const { name: rankId, seriesId: timeFlag } = param;
        setSelected({ rankId, timeFlag });
    };
    return <OverviewCom baseInfo={baseInfo} handleFilterChange={handleFilterChange} session={session}
        groupData={groupData} dataSource={dataSource} selected={selected}/>;
};
const OverviewCom = ({ baseInfo, handleFilterChange, groupData, dataSource, selected, session }: any): JSX.Element => {
    return <div className={'text-selectable'}
        style={{ textAlign: 'left', padding: '0 20px', overflow: 'auto', height: '100%' }}>
        <BaseInfo data={baseInfo} session={session}/>
        <div>
            <div>
                <div className={'common-title-bottom'}>Computation/Communication Overview{hit}</div>
                <Filter handleFilterChange={handleFilterChange} groupData={groupData}/>
                <div id={'overview-chart'} style={{ height: '400px' }} ></div>
            </div>
            <div style={{ padding: '0 3rem' }}>
                <SummaryTable dataSource={dataSource} style={{ display: 'none' }}/>
                <StatisticsTable {...selected}/>
            </div>
        </div>
    </div>;
};

export default ComputationCommunicationOverview;
