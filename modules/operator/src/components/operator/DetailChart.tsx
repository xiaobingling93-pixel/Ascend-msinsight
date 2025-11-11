/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type * as echarts from 'echarts';
import {
    getAdaptiveEchart, disposeAdaptiveEchart, chartVisbilityListener, chartColors, getDefaultChartOptions, customConsole as console,
} from '@insight/lib/utils';
import type { ConditionType } from './Filter';
import { queryOperatorCategory, queryOperatorComputeUnit } from '../RequestUtils';
import type { Session } from '../../entity/session';
import { themeInstance } from '../../theme/theme';
import styled from '@emotion/styled';

const ChartsContainer = styled.div`
  display: flex;
  align-items: center;
  height: 500px;
  padding: 0 24px;
  margin-bottom: 24px;
  gap: 24px;

  & .chart-item{
    width: 50%;
    padding: 16px 24px;
    height: 100%;
    border: 1px solid ${(props): string => props.theme.borderColor};
  }
`;

export type dataType = Array<{
    [name: string]: any;
    name: string ;
    value: number;
}>;

function InitCharts({ data, domId, isDark, title }: {data: dataType; domId: string; isDark: boolean;title: string}): void {
    const chartDom = document.getElementById(domId);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    disposeAdaptiveEchart(chartDom);
    const myChart: echarts.ECharts = getAdaptiveEchart(chartDom);
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
        textStyle: isDark ? { color: '#ffffff' } : { color: '#4E5865' },
    };
    baseOption.legend = {
        ...baseOption.legend ?? {},
        textStyle: {
            width: 180,
            overflow: 'truncate',
            color: '#8D98AA',
        },
        pageTextStyle: isDark ? { color: '#dcdcdc' } : {},
        pageIconColor: isDark ? '#aaa' : '#414141',
        pageIconInactiveColor: isDark ? '#414141' : '#aaa',
    };
    baseOption.tooltip = {
        ...baseOption.tooltip,
        ...getDefaultChartOptions(isDark).tooltip,
    };
    return baseOption;
};

const baseOption: echarts.EChartsOption = {
    textStyle: getDefaultChartOptions().textStyle,
    color: chartColors,
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
        left: 'left',
        top: 'middle',
        type: 'scroll',
        padding: [40, 5, 5, 5],
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
        return condition.group === 'Communication Operator Type' || condition.group === 'Communication Operator';
    };
    const updateOpTypeData = async (): Promise<void> => {
        const res = await queryOperatorCategory(condition).catch((err: any) => {
            console.error(err);
        });
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
        const res = await queryOperatorComputeUnit(condition).catch((err: any) => {
            console.error(err);
        });
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
    }, [JSON.stringify(condition)]);
    useEffect(() => {
        renderChart();
    }, [opTypeData, computeData]);
    useEffect(() => {
        renderChart();
    }, [themeInstance.currentTheme, t]);
    return (
        <ChartsContainer>
            <div className="chart-item">
                <div id={'opTypeChart'} style={{ height: '100%', width: '100%' }} ></div>
            </div>
            {isHideRight()
                ? <></>
                : <div className="chart-item">
                    <div id={'computeChart'} style={{ height: '100%', width: '100%' }}></div>
                </div>}
        </ChartsContainer>
    );
});

export default DetailChart;
