/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import * as echarts from 'echarts';
import { useTranslation } from 'react-i18next';
import { type IblockData } from './Index';
import { COLOR, getResizeEcharts, chartVisbilityListener, safeStr, sortFunc, chartColors } from 'ascend-utils';
import { LimitHit } from '../../LimitSet';

interface Iprops {
    blockId: string;
    data: IblockData[];
}

const baseOption = {
    color: chartColors,
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
        confine: true,
        formatter: function (params: any): string {
            return `${safeStr(params[0]?.name)} <br/>${params[0]?.marker} Cycles:${Number(params[0]?.data?.originValue)}`;
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
    const namelist = data.map(item => item.name);
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

const chartID = 'ComputeWorkload';
function ComputeWorkloadChart({ blockId, data }: Iprops): JSX.Element {
    const { t } = useTranslation('details');
    const [limit, setLimit] = useState({ maxSize: 5000, overlimit: true, current: 0 });
    const allData = useMemo(() => data.filter(item => item.blockId === blockId), [blockId, data]);
    const showData = useMemo(() => data.filter(item => item.blockId === blockId).slice(0, limit.maxSize), [blockId, data]);
    chartVisbilityListener(chartID, () => {
        InitCharts(showData);
    });

    useEffect(() => {
        setLimit({ ...limit, overlimit: allData.length > limit.maxSize, current: allData.length });
    }, [allData]);
    useEffect(() => {
        setTimeout(() => {
            InitCharts(showData);
        });
    }, [showData]);
    return (
        <div style={{ marginBottom: '20px' }}>
            {limit.overlimit && <LimitHit maxSize={limit.maxSize} name={`${t('Current Count')} (${limit.current})`}/>}
            <div id={chartID} style={{ height: '400px' }} ></div>
        </div>
    );
}

export default ComputeWorkloadChart;
