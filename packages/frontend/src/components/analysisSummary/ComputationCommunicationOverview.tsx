/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import * as echarts from 'echarts';
import React, { useState } from 'react';
import Filter, { ConditionDataType } from './Filter';
import StatisticsTable from './StatisticsTable';
import { VoidFunction } from '../../utils/interface';
import SummaryTable from './SummaryTable';
import { computationCommunicationData } from '../../utils/__test__/mockData';

interface SummaryDataType{
    rankId: string ;
    totalComputeTime: number;
    totalPureCommunicationTime: number;
    totalCommunicationNotOverLapTime: number;
    totalCommunicationTime: number;
    totalFreeTime: number;
    ComputeTimeRatio?: string | number;
    CommunicationTimeRatio?: string | number;
}

const queryTopData = async (conditions: ConditionDataType): Promise<SummaryDataType[]> => {
    const data: SummaryDataType[] = computationCommunicationData.top;
    if (window.request !== undefined) {
        const result = await window.request('test', {});
        return result.data;
    }
    return data;
};

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
            id: 'totalComputeTime',
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
            id: 'totalCommunicationNotOverLapTime',
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
            id: 'totalCommunicationTime',
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
            id: 'totalFreeTime',
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
    const order: Array<keyof SummaryDataType> = [ 'totalComputeTime', 'totalCommunicationNotOverLapTime',
        'totalCommunicationTime', 'totalFreeTime', 'ComputeTimeRatio', 'CommunicationTimeRatio' ];
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
    const [ dataSource, setDatasource ] = useState<SummaryDataType[]>([]);
    const [ selected, setSelected ] = useState({ rankId: '', timeFlag: '' });

    const handleFilterChange = async (conditions: any): Promise<void> => {
        const data = await queryTopData(conditions);
        setDatasource(data);
        initCharts(data, handleClick);
    };

    const handleClick = (param: any): void => {
        const { name: rankId, seriesId: timeFlag } = param;
        if (timeFlag !== 'totalFreeTime') {
            setSelected({ rankId, timeFlag });
        }
    };
    return <div style={{ textAlign: 'left', padding: '0 20px' }} className={'header-fixed-content-scroll'}>
        <div>
            <div className={'common-title-bottom'}>Computation/CommunicationOverview</div>
            <Filter handleFilterChange={handleFilterChange}/>
            <div id={'overview-chart'} style={{ height: '400px' }} ></div>
        </div>
        <div>
            <SummaryTable dataSource={dataSource}/>
            <StatisticsTable {...selected}/>
        </div>
    </div>;
};

export default ComputationCommunicationOverview;
