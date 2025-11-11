/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useEffect, useRef, useState } from 'react';
import { MIChart } from '@insight/lib/components';
import type { ChartsHandle } from '@insight/lib';
import { useTranslation } from 'react-i18next';
import { type EChartsOption } from 'echarts';
import { Session } from '../entity/session';
import { observer } from 'mobx-react';
import { type DetailData } from '../utils/RequestUtils';
import { getNewDetailData } from './dataHandler';
import { safeStr } from '@insight/lib/utils';

interface treemap {
    name: string;
    value: number;
    children?: treemap[];
};
const transformData = (node: DetailData, t: any, depth: number): treemap => {
    const convertedNode: treemap = {
        name: `${t(node.name)}`,
        value: node.size,
    };
    if (depth > 8) { return convertedNode; }
    if (node.subNodes && node.subNodes.length > 0) {
        convertedNode.children = node.subNodes.map(subNode => transformData(subNode, t, depth + 1));
    }
    return convertedNode;
};

const getColorGradientByGroup = (depth: number): string => {
    const baseHues = [210, 0, 30, 120];
    const group = depth % 4;
    const posInGroup = Math.floor(depth / 4);
    const saturation = 80;
    const lightnessStart = 30;
    const lightnessEnd = 70;
    const lightness = lightnessStart + ((lightnessEnd - lightnessStart) * posInGroup);
    return `hsl(${baseHues[group]}, ${saturation}%, ${lightness}%)`;
};

const mapLevelOption = (): any => {
    const levels = [];
    let i = 7;
    while (--i >= 0) {
        levels.push({
            itemStyle: {
                color: getColorGradientByGroup(i),
                borderColor: getColorGradientByGroup(i),
                borderWidth: 5,
                gapWidth: 5,
            },
            emphasis: {
                itemStyle: {
                    borderColor: '#ddd',
                },
            },
        });
    }
    return levels;
};

const getLevelOption = (): any => {
    return [
        {
            itemStyle: {
                color: '#fff',
                borderColor: '#fff',
                borderWidth: 5,
                gapWidth: 5,
            },
            upperLabel: {
                show: false,
            },
        },
        ...mapLevelOption(),
    ];
};

const getSeries = (chartData: treemap[], t: any): any => {
    return [{
        name: t('MemoryDisassembly'),
        type: 'treemap',
        visibleMin: 300,
        label: {
            show: true,
            formatter: (params: any): string => {
                const { name, value } = params.data;
                const isB: boolean = (value / 1024 / 1024).toFixed(0) === '0';
                return isB ? safeStr(`${name}:${value}B`) : safeStr(`${name}: ${(value / 1024 / 1024).toFixed(0)}MB`);
            },
        },
        upperLabel: {
            show: true,
            height: 30,
        },
        levels: getLevelOption(),
        data: chartData,
    }];
};
const MemorySliceChart: any = observer(({ session }: { session: Session }): React.ReactElement => {
    const { t } = useTranslation('leaks');
    const chartRef = useRef<ChartsHandle>(null);
    const [loading, setLoading] = useState(true);
    const [chartOptions, setChartOptions] = useState<EChartsOption>({});
    const [chartData, setChartData] = useState<treemap[]>([]);
    useEffect(() => {
        setChartOptions({
            xAxis: {
                show: false,
            },
            yAxis: {
                show: false,
            },
            legend: {
                show: false,
            },
            tooltip: {
                formatter: (params: any): string => {
                    const { name, value } = params.data;
                    const isB: boolean = (value / 1024 / 1024).toFixed(0) === '0';
                    return isB ? safeStr(`${name}:${value}B`) : safeStr(`${name}: ${(value / 1024 / 1024).toFixed(0)}MB`);
                },
            },
            series: getSeries(chartData, t) as echarts.SeriesOption,
        });
        setLoading(false);
    }, [chartData]);
    useEffect(() => {
        getNewDetailData(session);
    }, [session.memoryStamp, t]);
    useEffect(() => {
        setChartData([transformData(session.memoryData, t, 0)]);
    }, [session.memoryData, t]);
    return (
        <MIChart
            ref={chartRef}
            height="350px"
            width="calc(100vw - 120px)"
            loading={loading}
            options={chartOptions}
        />
    );
});
export default MemorySliceChart;
