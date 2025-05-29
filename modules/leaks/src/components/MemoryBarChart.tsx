/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useEffect, useMemo, useRef } from 'react';
import * as echarts from 'echarts';
import * as d3 from 'd3';
import { useTranslation } from 'react-i18next';
import { chartColors, getDefaultChartOptions, getLegendStyle, safeStr } from 'ascend-utils';
import { type Theme, useTheme } from '@emotion/react';
import type { BarData } from '../utils/RequestUtils';
const colorScale = d3.scaleOrdinal(d3.schemeCategory10);
interface InitParam {
    chartRef: React.RefObject<HTMLDivElement>;
    myChart: React.MutableRefObject<echarts.ECharts | null>;
    theme: Theme;
    blockData: BarData['blockData'];
    lineSource: number[][];
    source: number[][];
    isDark: boolean;
    t: any;
};
const getXAxis = (blockData: InitParam['blockData']): echarts.XAXisComponentOption => {
    return {
        type: 'value',
        min: blockData.minTimestamp,
        max: blockData.maxTimestamp,
        axisLine: {
            show: true,
        },
        axisTick: {
            show: false,
        },
        axisLabel: {
            formatter: function (value: number): string {
                return `${(value / 1000000).toFixed(3)}s`;
            },
        },
        splitLine: {
            show: false,
        },
    };
};
const getYAxis = (): echarts.YAXisComponentOption => {
    return {
        type: 'value',
        axisLine: {
            show: true,
        },
        axisTick: {
            show: false,
        },
        axisLabel: {
            formatter: function (value: number): string {
                return `${(value / 1024 / 1024).toFixed(3)}MB`;
            },
        },
        splitLine: {
            show: false,
        },
        scale: true,
    };
};
const getSeries = (t: InitParam['t'], source: InitParam['source'], lineSource: InitParam['lineSource']): any => {
    return ([
        {
            type: 'custom',
            name: t('MemoryBlocks'),
            renderItem: function (params: any, api: any): any {
                const categoryIndex = api.value(0);
                const points = source[categoryIndex].slice(9).reduce((acc: number[][], cur: number, index: number) => {
                    if (index % 2 === 0) {
                        acc.push([]);
                    }
                    acc[acc.length - 1].push(cur);
                    return acc;
                }, []).map(point => api.coord(point));
                points.push(points[0]);
                return {
                    type: 'polygon',
                    shape: {
                        points: points,
                    },
                    style: {
                        fill: colorScale(String(categoryIndex)),
                        stroke: 'black',
                        lineWidth: 0.03,
                    },
                };
            },
            data: source,
            encode: { x: [1, 3, 5, 7], y: [2, 4, 6, 8] },
            clip: true,
        },
        {
            type: 'line',
            name: t('MemoryAllocations'),
            data: lineSource,
            symbol: 'none',
            itemStyle: {
                color: 'rgba(255, 0, 0)',
            },
            lineStyle: {
                width: 1.5,
                color: 'rgba(255, 0, 0)',
            },
        },
    ]);
};
const getLegend = (t: InitParam['t'], theme: InitParam['theme']): echarts.LegendComponentOption => {
    return {
        data: [t('MemoryAllocations'), t('MemoryBlocks')],
        selected: {
            [t('MemoryAllocations')]: true,
            [t('MemoryBlocks')]: false,
        },
        ...getLegendStyle(theme),
    };
};
const init = ({ chartRef, myChart, theme, blockData, lineSource, source, isDark, t }: InitParam): void => {
    myChart.current = echarts.init(chartRef.current);
    const option = {
        textStyle: getDefaultChartOptions().textStyle,
        tooltip: {
            trigger: 'item',
            formatter: function (params: any): string {
                const info = blockData.blocks[params.dataIndex];
                return safeStr(`Addr: ${info.addr} Size: ${(info.size / 1024 / 1024).toFixed(3)}MB Life: ${((info.endTimestamp - info.startTimestamp) / 1000000).toFixed(3)}s`);
            },
            ...getDefaultChartOptions(isDark).tooltip,
        },
        xAxis: getXAxis(blockData),
        yAxis: getYAxis(),
        legend: getLegend(t, theme),
        dataZoom: [
            {
                type: 'slider',
                show: true,
                xAxisIndex: 0,
                start: 0,
                end: 100,
                filterMode: 'weakFilter',
            },
            {
                type: 'inside',
                xAxisIndex: 0,
            },
        ],
        series: getSeries(t, source, lineSource),
        toolbox: {
            feature: {
                dataZoom: {
                    yAxisIndex: false,
                    filterMode: 'weakFilter',
                },
                restore: {
                    show: true,
                },
            },
            right: 150,
            top: 10,
        },
        color: chartColors,
    };
    myChart.current?.setOption(option, { notMerge: true });
};
const MemoryBarChart: any = (data: BarData) => {
    const { t } = useTranslation('leaks');
    const theme = useTheme();
    const chartRef = React.useRef<HTMLDivElement>(null);
    const initBlockData = { blocks: [], minSize: 0, maxSize: 0, minTimestamp: 0, maxTimestamp: 0 };
    const initAllocData = { allocations: [], maxTimestamp: 0, minTimestamp: 0 };
    const { blockData = initBlockData, allocationData = initAllocData, isDark }: BarData = data;
    const lineSource = allocationData.allocations.map((line: any) => [line.timestamp, line.totalSize]);
    const source: number[][] = useMemo(() => {
        const blockSource: number[][] = [];
        blockData.blocks.forEach((block: any, index: number) => {
            const other: number[][] = [];
            block.path.forEach((item: number[]) => {
                other.unshift([item[0], item[1] + block.size]);
            });
            const arr = [...block.path, ...other];
            const length = block.path.length;
            const realSource = [index, block.path[0][0], block.path[0][1],
                block.path[length - 1][0], block.path[length - 1][1],
                other[0][0], other[0][1], other[other.length - 1][0], other[other.length - 1][1]];
            blockSource.push([...realSource, ...arr].flat());
        });
        return blockSource;
    }, [JSON.stringify(blockData.blocks)]);
    const myChart = useRef<echarts.ECharts | null>(null);
    const initParam = { chartRef, myChart, theme, blockData, lineSource, source, isDark, t };
    useEffect(() => {
        init(initParam);
        return () => {
            myChart.current?.dispose();
        };
    }, [blockData.blocks, allocationData.allocations, t]);
    return (
        <div>
            <div ref={chartRef} style={{ width: 1600, height: 350 }}></div>
        </div>
    );
};
export default MemoryBarChart;
