/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect } from 'react';
import * as echarts from 'echarts';
import { addResizeEvent, COLOR, Container } from '../Common';

function InitCharts(data: dataType): void {
    const chartDom = document.getElementById('main');
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    echarts.init(chartDom).dispose();
    const myChart = echarts.init(chartDom);
    myChart.setOption(wrapData(data));
    addResizeEvent(myChart);
}
function wrapData(data: dataType): any {
    baseOption.xAxis[0].data = data.rank_id;
    const order: Array<keyof dataType> = [ 'elapse_time', 'transit_time', 'synchronization_time',
        'wait_time', 'synchronization_time_ratio', 'wait_time_ratio' ];
    for (let i = 0; i < 6; i++) {
        baseOption.series[i].data = data[order[i]];
    }
    return baseOption;
}

const baseOption: any = {
    tooltip: {
        trigger: 'axis',
        axisPointer: {
            type: 'cross',
            crossStyle: {
                color: COLOR.BrightBlue,
                type: 'solid',
            },
        },
    },
    toolbox: {
        feature: {
            dataView: { show: true, readOnly: false },
            magicType: { show: true, type: [ 'line', 'bar' ] },
            restore: { show: true },
        },
    },
    legend: {
        data: [
            { name: 'Elapse Time', textStyle: { color: COLOR.Grey50 } },
            { name: 'Transit Time', textStyle: { color: COLOR.Grey50 } },
            { name: 'Synchronization Time', textStyle: { color: COLOR.Grey50 } },
            { name: 'Wait Time', textStyle: { color: COLOR.Grey50 } },
            { name: 'Synchronization Time Ratio', textStyle: { color: COLOR.Grey50 } },
            { name: 'Wait Time Ratio', textStyle: { color: COLOR.Grey50 } } ],
        tooltip: {
            show: true,
            formatter: function () {
                const div = document.createElement('div');
                div.className = 'legend-tooltip';
                div.append('Click to Switch Chart Display and Hide');
                return div;
            },
        },
    },
    xAxis: [
        {
            type: 'category',
            data: [],
            axisPointer: {
                type: 'shadow',
            },
            axisLabel: {
                color: COLOR.Grey40,
            },
        },
    ],
    yAxis: [
        {
            type: 'value',
            name: 'Time(ms)',
            axisLabel: {
                formatter: '{value}',
                color: COLOR.Grey40,
            },
        },
        {
            type: 'value',
            name: 'Ratio',
            axisLabel: {
                formatter: '{value}',
                color: COLOR.Grey40,
            },
            splitLine: {
                lineStyle: {
                    color: COLOR.Grey20,
                    type: 'dashed',
                },
            },
        },
    ],
    series: [
        {
            name: 'Elapse Time',
            type: 'bar',
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + 'ms';
                },
            },
            data: [ ],
        },
        {
            name: 'Transit Time',
            type: 'bar',
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + 'ms';
                },
            },
            data: [ ],
        },
        {
            name: 'Synchronization Time',
            type: 'bar',
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + 'ms';
                },
            },
            data: [ ],
        },
        {
            name: 'Wait Time',
            type: 'bar',
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + 'ms';
                },
            },
            data: [],
        },
        {
            name: 'Synchronization Time Ratio',
            type: 'line',
            yAxisIndex: 1,
            tooltip: {
                valueFormatter: function (value: any) {
                    return value;
                },
            },
            data: [ ],
        },
        {
            name: 'Wait Time Ratio',
            type: 'line',
            yAxisIndex: 1,
            tooltip: {
                valueFormatter: function (value: any) {
                    return value;
                },
            },
            data: [ ],
        },
    ],
};

export interface dataType{
    ranklist: string[];
    ElapseTime?: number[];
    TransitTime?: number[];
    SynchronizationTime?: number[];
    WaitTime?: number[];
    SynchronizationTimeRatio?: number[];
    WaitTimeRatio?: number[];
    [name: string]: any;
}

const CommunicationTimeChart = observer(function (props: {dataSource: dataType;active: boolean}) {
    useEffect(() => {
        setTimeout(() => {
            if (props.active) {
                InitCharts(props.dataSource);
            }
        });
    }, [ props.dataSource, props.active ]);
    return (
        <Container
            title={'Visualized Communication Time'}
            content={<div id={'main'} style={{ height: '400px' }} ></div>}
        />
    );
});

export default CommunicationTimeChart;
