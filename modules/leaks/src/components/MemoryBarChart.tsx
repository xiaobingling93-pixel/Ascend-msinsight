/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useState, useEffect, useMemo, useRef } from 'react';
import * as echarts from 'echarts';
import * as d3 from 'd3';
import { useTranslation } from 'react-i18next';
import { MIChart } from 'ascend-components';
import type { ChartsHandle } from 'ascend-components/MIChart';
import { safeStr } from 'ascend-utils';
import { type EChartsOption } from 'echarts';
import type { BlockData } from '../utils/RequestUtils';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import { getBarNewData } from './dataHandler';

const colorScale = d3.scaleOrdinal(d3.schemeCategory10);
interface InitParam {
    session: any;
    blockData: BlockData;
    lineSource: number[][];
    source: number[][];
    t: any;
};
const getXAxis = (session: InitParam['session']): echarts.XAXisComponentOption => {
    return {
        type: 'value',
        min: session.maxTime,
        max: session.minTime,
        axisLine: {
            show: true,
            lineStyle: {
                type: 'solid',
            },
        },
        axisTick: {
            show: false,
        },
        axisLabel: {
            formatter: function (value: number): string {
                return `${(value / 1000000000).toFixed(3)}s`;
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
        axisPointer: {
            show: false,
        },
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
const getLegend = (t: InitParam['t']): echarts.LegendComponentOption => {
    return {
        data: [t('MemoryAllocations'), t('MemoryBlocks')],
    };
};
const getTooltip = (blockData: InitParam['blockData']): echarts.TooltipComponentOption => {
    return {
        trigger: 'item',
        formatter: function (params: any): string {
            const info = blockData.blocks[params.dataIndex];
            if (!info) {
                return '';
            }
            return safeStr(`Addr: ${info.addr} Size: ${(info.size / 1024 / 1024).toFixed(3)}MB Life: ${((info.endTimestamp - info.startTimestamp) / 1000000).toFixed(3)}s`);
        },
    };
};
const getOptions = ({ session, blockData, lineSource, source, t }: InitParam): EChartsOption => {
    return {
        tooltip: getTooltip(blockData),
        xAxis: getXAxis(session),
        yAxis: getYAxis(),
        legend: getLegend(t),
        series: getSeries(t, source, lineSource),
        toolbox: {
            show: true,
            feature: {
                dataZoom: {
                    yAxisIndex: false,
                    filterMode: 'weakFilter',
                    icon: {
                        back: 'none',
                    },
                },
                restore: {
                    show: true,
                },
                dataView: {
                    show: false,
                },
            },
            right: 150,
            top: 20,
        },
        axisPointer: {
            show: true,
        },
    };
};
const MemoryBarChart = observer(({ session, setBarIns }: { session: any; setBarIns: (value: echarts.ECharts | null) => void }): React.ReactElement => {
    const { t } = useTranslation('leaks');
    const chartRef = useRef<ChartsHandle>(null);
    const [loading, setLoading] = useState(false);
    const [chartOptions, setChartOptions] = useState<EChartsOption>({});
    const { blockData, allocationData, deviceId, eventType, threadId } = session;
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
    const initParam: InitParam = { session, blockData, lineSource, source, t };
    useEffect(() => {
        if (deviceId !== '') {
            setLoading(true);
            getBarNewData(session);
        }
    }, [deviceId, eventType, threadId]);
    useEffect(() => {
        const param: EChartsOption = getOptions(initParam);
        setChartOptions(param);
        if (chartRef.current !== null && chartRef.current !== undefined) {
            setBarIns(chartRef.current?.getInstance());
        }
        setLoading(false);
        chartRef.current?.getInstance()?.getZr().off('click');
        chartRef.current?.getInstance()?.getZr().on('click', (params) => {
            if (params.target?.constructor?.name === 'ECPolyline' || params.target?.constructor?.name === 'Polygon' ||
                params.target?.constructor?.name === undefined) {
                const pointInPixel = [params.offsetX, params.offsetY];
                const pointInGrid = chartRef.current?.getInstance()?.convertFromPixel({ seriesIndex: 0 }, pointInPixel);
                const xValue = pointInGrid?.[0];
                if (xValue && xValue >= session.minTime && xValue <= session.maxTime) {
                    runInAction(() => {
                        session.memoryStamp = Number(xValue?.toFixed(0));
                    });
                }
            }
        });
    }, [threadId, JSON.stringify(blockData), session.maxTime, session.minTime, t]);
    useEffect(() => {
        chartRef.current?.getInstance()?.dispatchAction({
            type: 'takeGlobalCursor',
            key: 'dataZoomSelect',
            dataZoomSelectActive: true,
        });
    }, [chartOptions]);
    return (
        <MIChart
            ref={chartRef}
            width={'calc(100vw - 80px)'}
            height={'350px'}
            loading={loading}
            options={chartOptions}
            onEvents={
                {
                    datazoom: (params): void => {
                        const { startValue, endValue } = params.batch[0];
                        getBarNewData(session, Math.floor(startValue), Math.ceil(endValue));
                    },
                    restore: (): void => {
                        getBarNewData(session);
                    },
                }
            }
        />
    );
});
export default MemoryBarChart;
