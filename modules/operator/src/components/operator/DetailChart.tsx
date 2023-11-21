/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import * as echarts from 'echarts';
import { LeftRightContainer, addResizeEvent } from '../Common';
import { ConditionType } from './Filter';
import { queryOperatorCategory, queryOperatorComputeUnit } from '../RequestUtils';

export type dataType = Array<{
    name: string ;
    value: number;
    [name: string]: any;
}>;
function InitCharts(data: dataType, domId: string): void {
    const chartDom = document.getElementById(domId);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    echarts.init(chartDom).dispose();
    const myChart = echarts.init(chartDom);
    myChart.setOption(wrapData(data, domId));
    addResizeEvent(myChart);
}
function wrapData(data: dataType, domId: string): any {
    if (domId === 'opTypeChart') {
        baseOption.title = {
            text: '算子类型耗时占比', left: 'left',
        };
    } else {
        baseOption.title = {
            text: '计算单元耗时占比', left: 'left',
        };
    }
    (baseOption.series as echarts.SeriesOption[])[0].data = data;
    return baseOption;
}

const baseOption: echarts.EChartsOption = {
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

const DetailChart = observer(function ({ condition }: {condition: ConditionType}) {
    const [ opTypeData, setOpTypeData ] = useState([]);
    const [ computeData, setComputeData ] = useState([]);
    const updateData = (): void => {
        // 算子类型耗时占比：饼图
        updateOpTypeData();
        // 计算单元占比：饼图
        updateComputeData();
    };
    const updateOpTypeData = async (): Promise<void> => {
        const res = await queryOperatorCategory(condition);
        setOpTypeData(res);
    };
    const updateComputeData = async (): Promise<void> => {
        const res = await queryOperatorComputeUnit(condition);
        setComputeData(res);
    };

    useEffect(() => {
        updateData();
    }, [condition]);
    useEffect(() => {
        setTimeout(() => {
            InitCharts(opTypeData, 'opTypeChart');
            InitCharts(computeData, 'computeChart');
        });
    }, [ opTypeData, computeData ]);
    return (
        <LeftRightContainer
            style={{ height: '500px', padding: '20px 20px 0' }}
            left={<div id={'opTypeChart'} style={{ height: '100%' }} ></div>}
            right={<div id={'computeChart'} style={{ height: '100%' }} ></div>}
        />
    );
});

export default DetailChart;
