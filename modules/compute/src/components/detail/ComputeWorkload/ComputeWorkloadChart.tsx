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
import React, { useEffect, useMemo, useState } from 'react';
import * as echarts from 'echarts';
import { useTranslation } from 'react-i18next';
import { type IblockData } from './Index';
import { COLOR, getAdaptiveEchart, chartVisbilityListener, safeStr, sortFunc, chartColors, getDefaultChartOptions } from '@insight/lib/utils';
import { LimitHit } from '../../LimitSet';
import { CompareData } from '../../../utils/interface';
import { type Icondition } from './Filter';
import { cloneDeep } from 'lodash';

interface Iprops {
    condition: Icondition;
    data: Array<CompareData<IblockData>>;
}

interface SeriesData {
    value: string;
    originValue: string;
    source?: string;
}

interface Series {
    name: string;
    type: string;
    data: SeriesData[];
    itemStyle: { borderColor: string };
    barMaxWidth: number;
}

const baseOption = {
    textStyle: getDefaultChartOptions().textStyle,
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
        formatter: function (params: any[]): string {
            let result: string = `${safeStr(params[0]?.name)}`;
            params.forEach(param => {
                const source = param.data?.source;
                if (source !== undefined) {
                    result += `<br/>${param?.marker} Cycles ${safeStr(param?.data?.source)}:${Number(param?.data?.originValue)}`;
                } else {
                    result += `<br/>${param?.marker} Cycles:${Number(param?.data?.originValue)}`;
                }
            });
            return result;
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
            name: 'compare',
            type: 'bar',
            data: [] as unknown[],
            itemStyle: {
                borderColor: 'white',
            },
            barMaxWidth: 30,
        },
    ],
};

const defaultSeries: Series = {
    name: 'compare',
    type: 'bar',
    data: [],
    itemStyle: {
        borderColor: 'white',
    },
    barMaxWidth: 30,
};

function InitCharts(data: Array<CompareData<IblockData>>, isCompared: boolean): void {
    const chartDom = document.getElementById(chartID);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    const myChart: echarts.ECharts = getAdaptiveEchart(chartDom);
    myChart.setOption(wrapData(data, isCompared), { replaceMerge: ['series'] });
}

function wrapData(data: Array<CompareData<IblockData>>, isCompared: boolean): any {
    const option = cloneDeep(baseOption);
    data.sort((a, b) => sortFunc(a.compare.value, b.compare.value));
    const namelist = data.map(item => item.compare.name);
    option.yAxis.data = namelist;
    option.series = [];
    if (isCompared) {
        const compareValueList = data.map(item => ({ value: item.compare.value, originValue: item.compare.originValue, source: 'Compare' } as SeriesData));
        const baselineValueList = data.map(item => ({ value: item.baseline.value, originValue: item.baseline.originValue, source: 'Baseline' }));
        const compare: Series = cloneDeep(defaultSeries);
        const baseline: Series = cloneDeep(defaultSeries);
        baseline.name = 'baseline';
        compare.data = compareValueList;
        baseline.data = baselineValueList;
        option.series.push(compare);
        option.series.push(baseline);
    } else {
        const valueList = data.map(item => ({ value: item.compare.value, originValue: item.compare.originValue } as SeriesData));
        const compare: Series = cloneDeep(defaultSeries);
        compare.data = valueList;
        option.series.push(compare);
    }
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
function ComputeWorkloadChart({ condition, data }: Iprops): JSX.Element {
    const { t } = useTranslation('details');
    const [limit, setLimit] = useState({ maxSize: 5000, overlimit: true, current: 0 });
    const allData = useMemo(() => data.filter(item => item.compare.blockId === condition.blockId), [condition.blockId, data]);
    const showData = useMemo(() => data.filter(item => item.compare.blockId === condition.blockId).slice(0, limit.maxSize), [condition.blockId, data]);
    chartVisbilityListener(chartID, () => {
        InitCharts(showData, condition.isCompared);
    });

    useEffect(() => {
        setLimit({ ...limit, overlimit: allData.length > limit.maxSize, current: allData.length });
    }, [allData]);
    useEffect(() => {
        setTimeout(() => {
            InitCharts(showData, condition.isCompared);
        });
    }, [showData, condition.isCompared]);
    return (
        <div style={{ marginBottom: '20px' }}>
            {limit.overlimit && <LimitHit maxSize={limit.maxSize} name={`${t('Current Count')} (${limit.current})`}/>}
            <div id={chartID} style={{ height: '400px' }} ></div>
        </div>
    );
}

export default ComputeWorkloadChart;
