/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import * as echarts from 'echarts';
import { LeftRightContainer, addResizeEvent, chartVisbilityListener } from '../Common';
import { ConditionType } from './Filter';
import { queryOperatorCategory, queryOperatorComputeUnit } from '../RequestUtils';
import { Session } from '../../entity/session';

export type dataType = Array<{
    name: string ;
    value: number;
    [name: string]: any;
}>;
function InitCharts({ data, domId, isDark, title }: {data: dataType; domId: string; isDark: boolean;title: string}): void {
    const chartDom = document.getElementById(domId);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    echarts.init(chartDom).dispose();
    const myChart = echarts.init(chartDom);
    myChart.setOption(wrapData({ data, domId, isDark, title }));
    addResizeEvent(myChart);
}
function wrapData({ data, domId, isDark, title }: {data: dataType; domId: string; isDark: boolean;title: string}): any {
    const option = getOption({ isDark, title });
    (option.series as echarts.SeriesOption[])[0].data = data;
    return option;
}

const getOption = ({ isDark, title }: { isDark: boolean;title: string }): echarts.EChartsOption => {
    baseOption.title = {
        ...baseOption.title ?? {},
        text: title,
        textStyle: isDark ? { color: '#dcdcdc' } : {},
    };
    baseOption.legend = {
        ...baseOption.legend ?? {},
        textStyle: isDark ? { color: '#dcdcdc' } : {},
    };
    return baseOption;
};

const baseOption: echarts.EChartsOption = {
    title: {
        textStyle: { },
        left: 'left',
    },
    tooltip: {
        trigger: 'item',
    },
    legend: {
        orient: 'vertical',
        left: 'right',
        top: 'middle',
    },
    series: [
        {
            name: 'Operator',
            type: 'pie',
            radius: '50%',
            label: {
                show: true,
                position: 'inside',
                formatter(param) {
                    // correct the percentage
                    return `${param.percent}%`;
                },
            },
            data: [],
            emphasis: {
                itemStyle: {
                    shadowBlur: 10,
                    shadowOffsetX: 0,
                    shadowColor: 'rgba(0, 0, 0, 0.5)',
                },
            },
        },
    ],
};

// eslint-disable-next-line max-lines-per-function
const DetailChart = observer(function ({ condition, session }: {condition: ConditionType;session: Session}) {
    const [ opTypeData, setOpTypeData ] = useState([]);
    const [ computeData, setComputeData ] = useState([]);
    const updateData = (): void => {
        if (condition.rankId === '') {
            setOpTypeData([]);
            setComputeData([]);
            return;
        }
        // 算子类型耗时占比：饼图
        updateOpTypeData();
        // 计算单元占比：饼图
        updateComputeData();
    };
    const updateOpTypeData = async (): Promise<void> => {
        const res = await queryOperatorCategory(condition);
        if (res === null || res === undefined) {
            return;
        }
        const data = res.data.map((item: any) => ({ name: item.name, value: item.duration }));
        setOpTypeData(data);
    };
    const updateComputeData = async (): Promise<void> => {
        const res = await queryOperatorComputeUnit(condition);
        if (res === null || res === undefined) {
            return;
        }
        const data = res.data.map((item: any) => ({ name: item.name, value: item.duration }));
        setComputeData(data);
    };
    function renderChart(): void {
        InitCharts({ data: opTypeData, domId: 'opTypeChart', isDark: session.isDark, title: `Total Time(us) Group by ${condition.group}` });
        InitCharts({ data: computeData, domId: 'computeChart', isDark: session.isDark, title: 'Total Time(us) Group by Accelerator Core' });
    }
    // 避免echarts渲染空白
    chartVisbilityListener('opTypeChart', () => {
        renderChart();
    });
    useEffect(() => {
        updateData();
    }, [condition]);
    useEffect(() => {
        renderChart();
    }, [ opTypeData, computeData ]);
    useEffect(() => {
        renderChart();
    }, [session.isDark]);
    return (
        <LeftRightContainer
            style={{ height: '500px', padding: '20px 20px 0' }}
            left={<div id={'opTypeChart'} style={{ height: '100%' }} ></div>}
            right={<div id={'computeChart'} style={{ height: '100%' }} ></div>}
        />
    );
});

export default DetailChart;
