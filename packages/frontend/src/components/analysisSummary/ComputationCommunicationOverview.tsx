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
import BaseInfo, { defaultBaseInfo } from './BaseInfo';

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
            max: 100,
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
    baseOption.xAxis[0].data = data.map(item => item.rankId);
    const order: Array<keyof SummaryDataType> = [ 'computingTime', 'communicationNotOverLappedTime',
        'communicationOverLappedTime', 'freeTime', 'ComputeTimeRatio', 'CommunicationTimeRatio' ];
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

const ComputationCommunicationOverview = (): JSX.Element => {
    const [ groupData, setGroupData ] = useState({ rankList: [], stepList: [], init: false });
    const [ dataSource, setDatasource ] = useState<SummaryDataType[]>([]);
    const [ allDataSource, setAllDatasource ] = useState<SummaryDataType[]>([]);
    const [ selected, setSelected ] = useState({ rankId: '', timeFlag: '' });
    const [ baseInfo, setBaseInfo ] = useState(defaultBaseInfo);

    useEffect(() => {
        handleFilterChange({ step: 'All', rankIds: [], orderBy: 'computingTime', top: 0 });
    }, [ ]);
    useEffect(() => {
        initCharts(dataSource, handleClick);
    }, [dataSource]);
    const handleFilterChange = async (conditions: ConditionDataType, doQuery?: boolean): Promise<void> => {
        if (doQuery === false) {
            let data = [...allDataSource];
            data = data.slice(0, conditions.top);
            setDatasource(data);
            return;
        }
        const res = await queryTopSummary(conditions);
        const { summaryList, rankList, stepList } = res.result;
        setDatasource(summaryList);
        setAllDatasource(summaryList);
        if (!groupData.init) {
            setGroupData({ rankList, stepList, init: true });
            setBaseInfo(res.result);
        }
    };

    const handleClick = (param: any): void => {
        const { name: rankId, seriesId: timeFlag } = param;
        if (timeFlag !== 'totalFreeTime') {
            setSelected({ rankId, timeFlag });
        }
    };
    return <div className={'text-selectable'}
        style={{ textAlign: 'left', padding: '0 20px', overflow: 'auto', height: '100%' }}>
        <BaseInfo data={baseInfo}/>
        <div>
            <div>
                <div className={'common-title-bottom'}>Computation/Communication Overview</div>
                <Filter handleFilterChange={handleFilterChange} groupData={groupData}/>
                <div id={'overview-chart'} style={{ height: '400px' }} ></div>
            </div>
            <div>
                <SummaryTable dataSource={dataSource} style={{ display: 'none' }}/>
                <StatisticsTable {...selected}/>
            </div>
        </div>
    </div>;
};

export default ComputationCommunicationOverview;
