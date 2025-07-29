/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useEffect, useRef, useState } from 'react';
import { MIChart } from 'ascend-components';
import type { ChartsHandle } from 'ascend-components/MIChart';
import {
    queryFwpBwdTimeline,
    type QueryFwpBwdTimelineRes,
} from '../../utils/RequestUtils';
import type { EChartsOption, CustomSeriesRenderItem } from 'echarts';
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

enum OPERATOR_TYPE {
    FP_BP = 'FP/BP',
    P2P_OP = 'P2P Op',
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

export const colorPalette: Array<keyof Theme['colorPalette']> = [
    'deepBlue',
    'limeGreen',
    'sunsetOrange',
    'coralRed',
    'tealGreen',
    'royalPurple',
    'aquaBlue',
    'raspberryPink',
    'vividBlue',
    'vividRed',
    'skyBlue',
    'amethystPurple',
];

const colorMapping = new Map<string, number>();

export const hashToNumber = (input: string, maxIndex: number): number => {
    if (!colorMapping.has(input)) {
        colorMapping.set(input, colorMapping.size);
    }
    if (maxIndex === 0) {
        return 0;
    }
    return (colorMapping.get(input) ?? 0) % maxIndex;
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

const formatData = (dataSource: QueryFwpBwdTimelineRes, theme: Theme): {ranks: Ranks; data: SeriesDataItem[]; flowData: SeriesDataItem[]} => {
    const { rankList, flowList } = dataSource ?? {};
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
                        color: theme.colorPalette[colorPalette[hashToNumber(cname, colorPalette.length)]],
                    },
                });
            });
        });
    });
    const flowData: SeriesDataItem[] = flowList.map((flow) => {
        return {
            name: '',
            value: [flow[0].rankId, nsToMs(flow[0].startTime), flow[1].rankId, nsToMs(flow[1].startTime)],
            itemStyle: {
                color: theme.colorPalette.tealGreen,
            },
        };
    });

    return { ranks, data, flowData };
};

const renderLine: CustomSeriesRenderItem = (params, api) => {
    const start = api.coord([api.value(1), api.value(0)]);
    const end = api.coord([api.value(3), api.value(2)]);
    const height = ((api.size?.([0, 1]) as number[])?.[1] ?? 0) * 0.3;
    // 计算偏移量，使线条从算子中间开始
    const offset = height / 2;
    const startRectY = Math.round(start[1] + offset);
    const endRectY = Math.round(end[1] + offset);
    // 计算贝塞尔曲线的控制点
    const controlPoint = [
        (start[0] + end[0]) / 2, // 控制点X，位于起始点和终点的中间
        Math.min(start[1], end[1]) - height / 1.5, // 控制点Y，位于起始点和终点之上
    ];

    // 绘制贝塞尔曲线
    const bezierCurve = {
        x1: start[0],
        y1: startRectY,
        x2: end[0],
        y2: endRectY,
        cpx1: controlPoint[0],
        cpy1: controlPoint[1],
    };

    return {
        type: 'group',
        children: [
            {
                type: 'bezierCurve',
                transition: ['shape'],
                shape: bezierCurve,
                style: {
                    stroke: api.visual('color'),
                    fill: 'none',
                    lineWidth: 1,
                },
            },
        ],
    };
};

const renderRect: CustomSeriesRenderItem = (params, api) => {
    const { start: vStart, end: vEnd, component: vComponent, name: vName } = valueMap;
    const categoryIndex = api.value(0);
    const name = removePrefix(api.value(vName) as string);
    const start = api.coord([api.value(vStart), categoryIndex]);
    const end = api.coord([api.value(vEnd), categoryIndex]);
    const gridItemHeight = (api.size?.([0, 1]) as number[])[1];
    const offset = gridItemHeight * 0.04;

    const rectWidth = end[0] - start[0];
    const rectHeight = gridItemHeight * 0.3;
    const rectY = api.value(vComponent) === OPERATOR_TYPE.FP_BP ? Math.round(start[1] - rectHeight - offset) : Math.round(start[1] + offset);

    const textWidth = rectWidth > TEXT_PADDING_X ? Math.floor(rectWidth - TEXT_PADDING_X) : rectWidth;

    return {
        type: 'rect',
        transition: ['shape'],
        shape: {
            x: start[0],
            y: rectY,
            width: rectWidth,
            height: rectHeight,
        },
        textContent: {
            type: 'text',
            style: {
                text: name,
                fill: '#ffffff',
                overflow: 'truncate',
                width: textWidth,
            },
        },
        textConfig: {
            position: 'inside',
            inside: true,
            local: true,
        },
        style: {
            fill: api.visual('color'),
        },
    };
};

const baseOptions: EChartsOption = {
    tooltip: {
        formatter: function (params: any): string {
            return `
            <div class="formatter">
                <div class="row">${params.marker} ${safeStr(params.name)}</div>
                <div class="row">
                    <div class="label">Rank ID</div>
                    <div class="value">${safeStr(params.value[0])}</div>
                </div>
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
            moveOnMouseMove: 'ctrl',
            moveOnMouseWheel: 'shift',
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
        {
            type: 'custom',
            renderItem: renderLine,
            itemStyle: {
                opacity: 0.8,
            },
            encode: {
                x: [1, 3],
                y: [0, 2],
            },
            clip: true,
        },
    ],
};

interface FlowChartProps {
    step: string;
    stage: string;
    clusterPath: string;
}

export const FlowChart = (props: FlowChartProps): JSX.Element => {
    const { step, stage, clusterPath } = props;
    const chartRef = useRef<ChartsHandle>(null);
    const [chartOptions, setChartOptions] = useState<EChartsOption>({});
    const theme = useTheme();
    const scrollContainer = document.querySelector('.mi-page-content');
    const canvasEl = chartRef.current?.getChartDom()?.querySelector('canvas');
    const [loading, setLoading] = useState(true);
    const [chartHeight, setChartHeight] = useState('400px');

    const syncScroll = (e: WheelEvent): void => {
        if ((e.target as HTMLElement).tagName !== 'CANVAS') {
            return;
        }
        if (!e.ctrlKey && !e.shiftKey) {
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
                clusterPath,
            }).finally(() => {
                setLoading(false);
            });
            const { data, ranks, flowData } = formatData(dataSource, theme);
            const height = Math.round(clamp((ranks.length * RANK_HEIGHT) + CHART_BASE_HEIGHT, 200, 1000));
            setChartHeight(`${height}px`);
            setChartOptions(merge({}, baseOptions, {
                yAxis: {
                    data: ranks,
                },
                series: [
                    { data },
                    { data: flowData },
                ],
            }));
        };

        fetchData();
    }, [step, stage, clusterPath]);

    // echarts配置项中设置zoomOnMouseWheel: 'ctrl' 后, 滚轮在图表上无法触发滚动，此处需要手动处理滚动
    useEffect(() => {
        const chartDom = chartRef.current?.getChartDom();

        chartDom?.addEventListener('wheel', syncScroll, true);

        return (): void => {
            chartDom?.removeEventListener('wheel', syncScroll, true);
        };
    }, []);

    return <MIChart ref={chartRef} height={chartHeight} loading={loading} options={chartOptions} />;
};
