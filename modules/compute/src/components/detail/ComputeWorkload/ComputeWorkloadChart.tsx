/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo } from 'react';
import * as echarts from 'echarts';
import { type IblockData } from './Index';
import { COLOR, getResizeEcharts, chartVisbilityListener } from 'lib/CommonUtils';
interface Iprops {
    blockId: string;
    data: IblockData[];
}

const baseOption = {
    title: {
        text: 'Pipe Utilization',
        textStyle: { color: COLOR.Grey50 },
        x: 'center',
    },
    tooltip: {
        trigger: 'axis',
        axisPointer: {
            type: 'shadow',
        },
        formatter: function (params: any): string {
            return `Cycles:${Number(params[0]?.data?.originValue)}`;
        },
    },
    legend: {
        show: false,
    },
    grid: {
        left: '120',
        right: '4%',
        bottom: '5%',
        containLabel: false,
    },
    xAxis: {
        type: 'value',
        boundaryGap: [0, 0.01],
        axisLabel: {
            formatter: '{value}%',
            color: COLOR.Grey40,
        },
    },
    yAxis: {
        type: 'category',
        axisLabel: {
            formatter: '{value}',
            color: COLOR.Grey40,
        },
        data: [] as string[],
    },
    series: [
        {
            name: 'Pipe Utilization',
            type: 'bar',
            data: [] as unknown[],
            itemStyle: {
                color: COLOR.LIGHT_BLUE,
                borderColor: 'white',
            },
            barMaxWidth: 30,
        },
    ],
};

let myChart: echarts.ECharts;
function InitCharts(data: IblockData[]): void {
    const chartDom = document.getElementById(chartID);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    myChart = getResizeEcharts(chartDom, myChart);
    myChart.setOption(wrapData(data));
}

function wrapData(data: IblockData[]): any {
    const option = { ...baseOption };
    data.sort((a, b) => sortFunc(a.value, b.value));
    const namelist = data.map(item => `${item.blockType?.toUpperCase()}_${item.name.replaceAll(' ', '_')}`);
    const valuelist = data.map(item => ({ value: item.value, originValue: item.originValue }));
    option.yAxis.data = namelist;
    option.series[0].data = valuelist;
    // 左边距
    let maxLength = 0;
    namelist.forEach(item => {
        if (item.length > maxLength) {
            maxLength = item.length;
        }
    });
    option.grid.left = String(maxLength * 9);
    return option;
}
function sortFunc<T>(a: T, b: T): number {
    const aNum = Number(a);
    const bNum = Number(b);
    if (isNaN(aNum)) {
        return -1;
    } else if (isNaN(bNum)) {
        return 1;
    } else {
        return aNum - bNum;
    }
}

const chartID = 'ComputeWorkload';
function ComputeWorkloadChart({ blockId, data }: Iprops): JSX.Element {
    const showData = useMemo(() => data.filter(item => item.blockId === blockId), [blockId, data]);
    chartVisbilityListener(chartID, () => {
        InitCharts(showData);
    });
    useEffect(() => {
        setTimeout(() => {
            InitCharts(showData);
        });
    }, [showData]);
    return (
        <div style={{ padding: '20px' }}>
            <div id={chartID} style={{ height: '400px' }} ></div>
        </div>
    );
}

export default ComputeWorkloadChart;
