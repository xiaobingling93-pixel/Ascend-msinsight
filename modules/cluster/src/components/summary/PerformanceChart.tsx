/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useEffect, useRef, useState } from 'react';
import { MIChart } from 'ascend-components';
import type { ChartsHandle } from 'ascend-components/MIChart';
import type { EChartsOption } from 'echarts';
import { merge } from 'lodash';
import { PerformanceDataMap, Session } from '../../entity/session';
import { PerformanceDataItem } from '../../utils/interface';
import { GenerateConditions } from '../communicatorContainer/CommunicatorContainer';
import { Advice } from 'ascend-utils';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';

const VALUE_ALL = 'All';

const baseOptions: EChartsOption = {
    legend: {
        type: 'scroll',
    },
    tooltip: {
        trigger: 'axis',
        axisPointer: {
            type: 'cross',
        },
    },
    xAxis: {
        type: 'category',
        name: 'Rank',
        data: [],
        nameLocation: 'start',
        nameGap: 15,
    },
    yAxis: [
        {
            type: 'value',
            name: 'Time(μs)',
        },
        {
            type: 'value',
            name: 'Ratio',
            min: 0,
            axisLabel: {
                formatter: '{value}%',
            },
        },
    ],
    series: [],
};

interface PerformanceChartProps extends GenerateConditions {
    session: Session;
    group: string;
    orderBy: string;
    top: string;
    setActiveRankId: (rankId: string) => void;
    loading: boolean;
    advices: string[];
}

const getSeries = (session: Session, datasource: PerformanceDataItem[], t: TFunction): any => {
    return session.performanceChartsIndicators?.map(indicator => {
        const { chart, key, name, stack, yAxisType, unit } = indicator;
        const data = datasource.map(item => item[key]);
        const yAxisIndex = yAxisType === 'time' ? 0 : 1;

        return {
            name: t(name),
            type: chart,
            stack,
            emphasis: {
                focus: 'series',
            },
            tooltip: {
                valueFormatter: function (value: string): string {
                    return `${value} ${unit}`;
                },
            },
            yAxisIndex,
            data,
        };
    });
};

const getLegend = (session: Session, t: TFunction): EChartsOption['legend'] => {
    const legendData: string[] = [];
    const legendSelected: Record<string, boolean> = {};

    session.performanceChartsIndicators.forEach(indicator => {
        const { name, visible } = indicator;
        const tName = t(name);
        // 默认显示的图例排在前面
        if (visible) {
            legendData.unshift(tName);
        } else {
            legendData.push(tName);
        }

        legendSelected[tName] = visible;
    });

    return {
        data: legendData,
        selected: legendSelected,
    };
};

export const PerformanceChart = observer((props: PerformanceChartProps): JSX.Element => {
    const { session, setActiveRankId, advices, top, group, orderBy, loading } = props;
    const chartRef = useRef<ChartsHandle>(null);
    const canvasEl = chartRef.current?.getChartDom()?.querySelector('canvas');
    const [chartOptions, setChartOptions] = useState<EChartsOption>({});
    const [datasource, setDatasource] = useState<PerformanceDataItem[]>([]);
    const { t } = useTranslation('summary');

    // 图表的默认宽为100，此处是fix图表初始化时宽度未撑开的问题
    if (canvasEl?.width === 100) {
        chartRef.current?.getInstance()?.resize();
    }

    const filterData = (performanceData: PerformanceDataItem[], performanceDataMap: PerformanceDataMap): PerformanceDataItem[] => {
        let result = performanceData;

        if (group !== VALUE_ALL) {
            const groupList = group.split(',');
            const emptyData: Omit<PerformanceDataItem, 'index'> = {};
            Object.keys(performanceData[0] ?? []).forEach(key => {
                if (key !== 'index') {
                    emptyData[key] = 0;
                }
            });
            result = groupList.map(item => {
                const index = Number(item);
                return performanceDataMap.get(index) ?? {
                    index,
                    ...emptyData,
                };
            });
        }

        result = result.slice(0, top === VALUE_ALL ? result.length : Number(top));

        result.sort((a, b) => {
            if (orderBy === 'rankId') {
                return a.index - b.index;
            }
            return a[orderBy] - b[orderBy];
        });

        return result;
    };

    useEffect(() => {
        const legend = getLegend(session, t);
        const series = getSeries(session, datasource, t);

        const options = merge({}, baseOptions, {
            legend,
            xAxis: {
                data: datasource.map(item => item.index),
            },
            series,
        });
        setChartOptions(options);
    }, [datasource, session.indicatorList, t]);

    useEffect(() => {
        const filteredData = filterData(session.performanceData, session.performanceDataMap);
        const firsRankId = filteredData[0]?.index;

        setDatasource(filteredData);
        if (firsRankId !== undefined) {
            setActiveRankId(firsRankId.toString());
        }
    }, [top, group, orderBy, session.performanceData]);

    return <>
        <MIChart
            width={'calc(100vw - 80px)'}
            ref={chartRef}
            loading={loading}
            options={chartOptions}
            onEvents={
                {
                    click: (e): void => {
                        setActiveRankId(e.name);
                    },
                }
            }
        />
        {
            advices.length > 0
                ? <div>
                    <Advice text={advices}/>
                </div>
                : <></>
        }
    </>;
});
