/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type * as echarts from 'echarts';
import { LeftRightContainer } from '../Common';
import { getResizeEcharts, chartVisbilityListener } from 'lib/CommonUtils';
import { ConditionType } from './Filter';
import { queryOperatorCategory, queryOperatorComputeUnit } from '../RequestUtils';
import { Session } from '../../entity/session';
import { themeInstance } from '../../theme/theme';

export type dataType = Array<{
    name: string ;
    value: number;
    [name: string]: any;
}>;

let myChart: echarts.ECharts;
function InitCharts({ data, domId, isDark, title }: {data: dataType; domId: string; isDark: boolean;title: string}): void {
    const chartDom = document.getElementById(domId);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    myChart = getResizeEcharts(chartDom, myChart);
    myChart.setOption(wrapData({ data, domId, isDark, title }));
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
        textStyle: {
            width: 180,
            overflow: 'truncate',
            color: isDark ? '#dcdcdc' : '#333',
        },
        pageTextStyle: isDark ? { color: '#dcdcdc' } : {},
        pageIconColor: isDark ? '#aaa' : '#414141',
        pageIconInactiveColor: isDark ? '#414141' : '#aaa',
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
        confine: true,
    },
    legend: {
        orient: 'vertical',
        left: 'right',
        top: 'middle',
        type: 'scroll',
    },
    series: [
        {
            name: 'Operator',
            type: 'pie',
            radius: '50%',
            label: {
                show: true,
                position: 'inside',
                formatter(param): string {
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
const DetailChart = observer(({ condition, session }: {condition: ConditionType;session: Session}) => {
    const [opTypeData, setOpTypeData] = useState([]);
    const [computeData, setComputeData] = useState([]);
    const { t } = useTranslation('operator', { keyPrefix: 'sessionTitle' });
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
    const isHideRight = (): boolean => {
        return condition.group === 'HCCL Operator Type' || condition.group === 'HCCL Operator';
    };
    const updateOpTypeData = async (): Promise<void> => {
        const res = await queryOperatorCategory(condition);
        if (res === null || res === undefined) {
            return;
        }
        const data = res.data.map((item: any) => ({ name: item.name, value: item.duration }));
        if (JSON.stringify(opTypeData) === JSON.stringify(data)) {
            return;
        }
        setOpTypeData(data);
    };
    const updateComputeData = async (): Promise<void> => {
        const res = await queryOperatorComputeUnit(condition);
        if (res === null || res === undefined) {
            return;
        }
        const data = res.data.map((item: any) => ({ name: item.name, value: item.duration }));
        if (JSON.stringify(computeData) === JSON.stringify(data)) {
            return;
        }
        setComputeData(data);
    };
    function renderChart(): void {
        const isDark = themeInstance.currentTheme === 'dark';
        InitCharts({ data: opTypeData, domId: 'opTypeChart', isDark, title: t('TotalTimeGroupByCriteria', { criteria: t(condition.group) }) });
        if (!isHideRight()) {
            InitCharts({ data: computeData, domId: 'computeChart', isDark, title: t('TotalTimeGroupByCriteria', { criteria: t('Accelerator Core') }) });
        }
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
    }, [opTypeData, computeData]);
    useEffect(() => {
        renderChart();
    }, [themeInstance.currentTheme, t]);
    return (
        <LeftRightContainer
            style={{ height: '500px', padding: '20px 20px 0' }}
            headerStyle={{ overflow: 'visible' }}
            bodyStyle={{ overflow: 'visible' }}
            left={<div id={'opTypeChart'} style={{ height: '100%', width: '100%' }} ></div>}
            right={isHideRight() ? <></> : <div id={'computeChart'} style={{ height: '100%', width: '100%' }} ></div>}
        />
    );
});

export default DetailChart;
