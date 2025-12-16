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

import { MIChart } from '@insight/lib/components';
import React, { useEffect, useRef, useState } from 'react';
import type { ChartsHandle } from '@insight/lib';
import type { EChartsOption } from 'echarts';
import { merge } from 'lodash';
import { clamp, safeStr } from '@insight/lib/utils';
import { QueryExpertHotspotItem } from '../../../utils/interface';

const RANK_HEIGHT = 14;
const CHART_BASE_HEIGHT = 200;

interface ExpertLoadBalancingChartProps {
    data: QueryExpertHotspotItem[];
    loading: boolean;
}

interface FormatDataReturns {
    data: Array<Array<number | string>>;
    expertIndexList: string[];
    layersDepthList: string[];
    minVisits: number;
    maxVisits: number;
}

const formatData = (res: QueryExpertHotspotItem[]): FormatDataReturns => {
    const expertIndexList = [...new Set(res.map((item) => item.expertIndex.toString()))];
    const layersDepthList = [...new Set(res.map((item) => item.layer.toString()))].sort((a, b) => Number(b) - Number(a));
    // 初始化最大值和最小值
    let maxVisits = res[0]?.visits ?? 0;
    let minVisits = res[0]?.visits ?? 0;
    for (const item of res) {
        const visits = item.visits;
        if (visits > maxVisits) {
            maxVisits = visits;
        }
        if (visits < minVisits) {
            minVisits = visits;
        }
    }

    const data = res.map((item) => {
        return [
            item.expertIndex.toString(),
            item.layer.toString(),
            item.visits,
            item.rankId,
            item.expertId,
        ];
    });

    return { data, expertIndexList, layersDepthList, minVisits, maxVisits };
};

const baseOptions: EChartsOption = {
    grid: {
        height: '80%',
        top: '5%',
        left: 80,
        right: 90,
    },
    tooltip: {
        formatter: function (params: any): string {
            const [expertIndex, layerDepth, visit, rankId, expertId] = params.data;
            return `
            <div class="formatter">
                <div class="row">
                    <div class="label">${params.marker}</div>
                </div>
                <div class="row">
                    <div class="label">Expert Index</div>
                    <div class="value">${safeStr(expertIndex)}</div>
                </div>
                <div class="row">
                    <div class="label">Expert Id</div>
                    <div class="value">${safeStr(expertId)}</div>
                </div>
                <div class="row">
                    <div class="label">Layer Depth</div>
                    <div class="value">${safeStr(layerDepth)}</div>
                </div>
                <div class="row">
                    <div class="label">Visits</div>
                    <div class="value">${safeStr(visit)}</div>
                </div>
                <div class="row">
                    <div class="label">Rank Id</div>
                    <div class="value">${safeStr(rankId)}</div>
                </div>
            </div>
            `;
        },
    },
    xAxis: {
        type: 'category',
        name: 'Expert Index',
        data: [],
        splitArea: {
            show: true,
        },
    },
    yAxis: {
        type: 'category',
        name: 'Layer Depth',
        data: [],
        splitArea: {
            show: true,
        },
    },
    visualMap: {
        textStyle: {
            color: '#8D98AA',
        },
        calculable: true,
        orient: 'horizontal',
        left: 'center',
        bottom: '3%',
        itemHeight: 300, // 长度
        dimension: 2,
        align: 'left',
    },
    dataZoom: [
        {
            type: 'inside',
            xAxisIndex: [0],
            zoomOnMouseWheel: 'ctrl',
        },
    ],
    series: [
        {
            type: 'heatmap',
            emphasis: {
                itemStyle: {
                    shadowBlur: 10,
                    shadowColor: 'rgba(0, 0, 0, 0.5)',
                },
            },
            encode: {
                x: 'x',
                y: 'y',
                value: 'value',
            },
        },
    ],
};

const syncScroll = (e: WheelEvent): void => {
    if ((e.target as HTMLElement).tagName !== 'CANVAS') {
        return;
    }
    if (!e.ctrlKey) {
        const scrollContainer = document.querySelector('.mi-page-content');
        scrollContainer?.scrollBy(0, e.deltaY);
    }
};

export const ExpertLoadBalancingChart = ({ loading, data: dataSource }: ExpertLoadBalancingChartProps): JSX.Element => {
    const chartRef = useRef<ChartsHandle>(null);
    const [chartOptions, setChartOptions] = useState<EChartsOption>({});
    const [chartHeight, setChartHeight] = useState('600px');

    const handleDataChange = (): void => {
        const { data, expertIndexList, layersDepthList, minVisits, maxVisits } = formatData(dataSource);
        const height = Math.round(clamp((layersDepthList.length * RANK_HEIGHT) + CHART_BASE_HEIGHT, 600, 1000));

        setChartHeight(`${height}px`);
        setChartOptions(merge({}, baseOptions, {
            xAxis: {
                data: expertIndexList,
            },
            yAxis: {
                data: layersDepthList,
            },
            visualMap: {
                min: minVisits,
                max: maxVisits,
            },
            series: [
                {
                    data,
                    label: {
                        show: expertIndexList.length < 32,
                    },
                },
            ],
        }));
    };

    useEffect(() => {
        handleDataChange();
    }, [dataSource]);

    useEffect(() => {
        const chartDom = chartRef.current?.getChartDom();

        chartDom?.addEventListener('wheel', syncScroll, true);

        return (): void => {
            chartDom?.removeEventListener('wheel', syncScroll, true);
        };
    }, []);

    return <MIChart
        ref={chartRef}
        width={'calc(100vw - 80px)'}
        height={chartHeight}
        loading={loading}
        options={chartOptions}
    />;
};
