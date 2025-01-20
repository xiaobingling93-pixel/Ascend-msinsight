/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useEffect, useRef, useState } from 'react';
import { MIChart } from 'ascend-components';
import type { ChartsHandle } from 'ascend-components/MIChart';
import {
    queryFwpBwdTimeline,
    type QueryFwpBwdTimelineRes,
    TraceItem,
} from '../../utils/RequestUtils';
import type { EChartsOption, CustomSeriesRenderItem } from 'echarts';
import * as echarts from 'echarts';
import { merge } from 'lodash';
import { type Theme, useTheme } from '@emotion/react';
import { clamp, safeStr } from 'ascend-utils';

const TEXT_PADDING = 4;
const TEXT_PADDING_X = TEXT_PADDING * 2;
const RANK_HEIGHT = 50;
const CHART_BASE_HEIGHT = 200;

type Ranks = string[];
interface SeriesDataItem {
    name: string;
    value: Array<string | number>;
    itemStyle: {
        color: string;
    };
}

const PREFIX = 'prefix_';

const addPrefix = (str: string, prefix = PREFIX): string => {
    return prefix + str;
};

const removePrefix = (str: string, prefix = PREFIX): string => {
    if (str.startsWith(prefix)) {
        return str.slice(prefix.length);
    }
    return str;
};

const nsToMs = (ns: number): number => {
    return ns / 1000000;
};

const colorMap: {[key in TraceItem['cname']]: keyof Theme['colorPalette']} = {
    FP: 'deepBlue',
    BP: 'limeGreen',
    SEND: 'sunsetOrange',
    RECV: 'tealGreen',
    BATCH_SEND_RECV: 'royalPurple',
};

//  指定维度的映射值
const valueMap = {
    rank: 0,
    start: 1,
    end: 2,
    duration: 3,
    component: 4,
    name: 5,
};

const formatData = (dataSource: QueryFwpBwdTimelineRes | null, theme: Theme): {ranks: Ranks; data: SeriesDataItem[]} => {
    const { rankList } = dataSource ?? {};
    const data: SeriesDataItem[] = [];
    const ranks = rankList?.map(rank => rank.rank).reverse() ?? [];

    rankList?.forEach((rank) => {
        rank.componentList.forEach((component) => {
            component.traceList.forEach((trace) => {
                const { name, cname, start, duration } = trace;
                // 由于value中name的值可能为数字类型的字符串‘0’，导致echarts取值时会转为number类型，导致其他字符串类型的值变为NaN,所以加前缀处理，防止echarts将其转为数字类型
                data.push({
                    name,
                    value: [rank.rank, nsToMs(start), nsToMs((start + duration)), nsToMs(duration), component.component, addPrefix(name)],
                    itemStyle: {
                        color: theme.colorPalette[colorMap[cname]],
                    },
                });
            });
        });
    });

    return { ranks, data };
};

interface CoordSys {
    x: number;
    y: number;
    width: number;
    height: number;
}
const renderRect: CustomSeriesRenderItem = (params, api) => {
    const { start: vStart, end: vEnd, name: vName } = valueMap;
    const categoryIndex = api.value(0);
    const name = removePrefix(api.value(vName) as string);
    const start = api.coord([api.value(vStart), categoryIndex]);
    const end = api.coord([api.value(vEnd), categoryIndex]);
    const gridItemHeight = (api.size?.([0, 1]) as number[])[1];
    const { x: coordSysX, y: coordSysY, width: coordSysWidth, height: coordSysHeight } = params.coordSys as unknown as CoordSys;

    const rectWidth = end[0] - start[0];
    const rectHeight = gridItemHeight * 0.6;
    const rectY = Math.round(start[1] - (rectHeight / 2));
    const rectShape = echarts.graphic.clipRectByRect(
        {
            x: start[0],
            y: rectY,
            width: rectWidth,
            height: rectHeight,
        },
        {
            x: coordSysX,
            y: coordSysY,
            width: coordSysWidth,
            height: coordSysHeight,
        },
    );

    const textWidth = rectWidth > TEXT_PADDING_X ? Math.floor(rectWidth - TEXT_PADDING_X) : rectWidth;

    return {
        type: 'group',
        children: [
            {
                type: 'rect',
                transition: ['shape'],
                shape: rectShape,
                style: {
                    fill: api.visual('color'),
                },
            },
            {
                type: 'text',
                style: {
                    text: name,
                    width: textWidth,
                    overflow: 'truncate',
                    x: start[0] + (rectWidth / 2),
                    y: rectY + (rectHeight / 2),
                    textAlign: 'center',
                    textVerticalAlign: 'center',
                    fill: '#ffffff',
                },
            },
        ],
    };
};

const baseOptions: EChartsOption = {
    tooltip: {
        formatter: function (params: any): string {
            return `
            <div class="formatter">
                <div class="row">${params.marker} ${safeStr(params.name)}</div>
                <div class="row">
                    <div class="label">Start Time</div>
                    <div class="value">${safeStr(params.value[1])} ms</div>
                </div>
                <div class="row">
                    <div class="label">Duration</div>
                    <div class="value">${safeStr(params.value[3])} ms</div>
                </div>
            </div>
            `;
        },
    },
    dataZoom: [
        {
            type: 'slider',
            filterMode: 'weakFilter',
            bottom: 10,
            labelFormatter: '',
            height: 20,
        },
        {
            type: 'inside',
            filterMode: 'weakFilter',
            zoomOnMouseWheel: 'ctrl',
        },
    ],
    xAxis: {
        name: 'Time(ms)',
        scale: true,
    },
    yAxis: {
        name: 'Rank ID',
    },
    series: [
        {
            type: 'custom',
            renderItem: renderRect,
            itemStyle: {
                opacity: 0.8,
            },
            encode: {
                x: [1, 2],
                y: 0,
            },
            clip: true,
        },
    ],
};

interface FlowChartProps {
    step: string;
    stage: string;
}

export const FlowChart = (props: FlowChartProps): JSX.Element => {
    const { step, stage } = props;
    const chartRef = useRef<ChartsHandle>(null);
    const [chartOptions, setChartOptions] = useState<EChartsOption>({});
    const theme = useTheme();
    const scrollContainer = document.querySelector('.mi-page-content');
    const canvasEl = chartRef.current?.getChartDom()?.querySelector('canvas');
    const [loading, setLoading] = useState(true);
    const [chartHeight, setChartHeight] = useState('400px');

    const syncScroll = (e: WheelEvent): void => {
        if (!e.ctrlKey) {
            scrollContainer?.scrollBy(0, e.deltaY);
        }
    };

    // 图表的默认宽为100，此处是fix图表初始化时宽度未撑开的问题
    if (canvasEl?.width === 100) {
        chartRef.current?.getInstance()?.resize();
    }

    useEffect(() => {
        const fetchData = async (): Promise<void> => {
            setLoading(true);
            if (!step || !stage) {
                return;
            }
            const dataSource = await queryFwpBwdTimeline({
                stepId: step,
                stageId: stage,
            }).finally(() => {
                setLoading(false);
            });
            const { data, ranks } = formatData(dataSource, theme);
            const height = Math.round(clamp((ranks.length * RANK_HEIGHT) + CHART_BASE_HEIGHT, 200, 1000));
            setChartHeight(`${height}px`);
            setChartOptions(merge({}, baseOptions, {
                yAxis: {
                    data: ranks,
                },
                series: [
                    { data },
                ],
            }));
        };

        fetchData();
    }, [step, stage]);

    // echarts配置项中设置zoomOnMouseWheel: 'ctrl' 后, 滚轮在图表上无法触发滚动，此处需要手动处理滚动
    useEffect(() => {
        canvasEl?.addEventListener('wheel', syncScroll);

        return (): void => {
            canvasEl?.removeEventListener('wheel', syncScroll);
        };
    }, [canvasEl]);

    return <MIChart ref={chartRef} height={chartHeight} loading={loading} options={chartOptions} />;
};
