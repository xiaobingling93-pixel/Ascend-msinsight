/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useEffect, useState, useRef } from 'react';
import * as echarts from 'echarts';
import { MIChart } from 'ascend-components';
import type { ChartsHandle } from 'ascend-components/MIChart';
import { type EChartsOption } from 'echarts';
import { Session } from '../entity/session';
import { observer } from 'mobx-react';
import { getFuncNewData } from './dataHandler';
const colorTypes: string[] = ['#8fd3e8', '#d95850', '#eb8146', '#ffb248', '#f2d643', '#ebdba4', '#fcce10', '#b5c334', '#1bca93'];
const transData = (data: any): any => {
    return data.map((item: any, index: number) => ({
        name: item.func,
        value: [item.depth, item.startTimestamp, item.endTimestamp, item.func],
        itemStyle: {
            color: colorTypes[index % 9],
        },
    }));
};
const getToolbox = (): echarts.ToolboxComponentOption => {
    return {
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
    };
};
const getSeries = (session: Session): any => {
    return [
        {
            type: 'custom',
            renderItem: (params: any, api: any): echarts.CustomSeriesRenderItemReturn => {
                const level = api.value(0);
                const start = api.coord([api.value(1), level]);
                const end = api.coord([api.value(2), level]);
                const height = ((api.size([0, 1]) || [0, 20]) as number[])[1];
                const width = end[0] - start[0];
                return {
                    type: 'rect',
                    transition: ['shape'],
                    shape: {
                        x: start[0],
                        y: start[1] - (height / 2),
                        width,
                        height: height - 2,
                        r: 2,
                    },
                    style: { fill: api.visual('color') },
                    emphasis: { style: { stroke: '#000' } },
                    textConfig: { position: 'insideLeft' },
                };
            },
            encode: {
                x: [0, 1, 2],
                y: 0,
            },
            data: transData(session.funcData.traces),
            clip: true,
        },
    ];
};
const getOptions = (session: Session): EChartsOption => {
    return {
        color: ['#5470c6', '#91cc75', '#fac858', '#ee6666', '#73c0de', '#3ba272', '#fc8452', '#9a60b4', '#ea7ccc'],
        gradientColor: ['#f6efa6', '#d88273', '#bf444c'],
        xAxis: {
            axisTick: {
                show: false,
            },
            min: session.maxTime,
            max: session.minTime,
            axisLine: {
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
        },

        yAxis: {
            show: false,
            inverse: true,
            axisPointer: {
                show: false,
                snap: true,
            },
        },
        toolbox: getToolbox(),
        axisPointer: {
            show: true,
        },
        series: getSeries(session),
    };
};

const MemoryFunctionCall = observer(({ session, setFuncIns }: {
    session: Session;
    setFuncIns: (value: echarts.ECharts | null) => void;
}): React.ReactElement => {
    const chartRef = useRef<ChartsHandle>(null);
    const [loading, setLoading] = useState(false);
    const [chartOptions, setChartOptions] = useState<EChartsOption>({});
    useEffect(() => {
        if (session.deviceId !== '') {
            setLoading(true);
            getFuncNewData(session);
        }
    }, [session.deviceId, session.eventType, session.threadId]);
    useEffect(() => {
        setChartOptions(getOptions(session));
        if (chartRef.current !== null && chartRef.current !== undefined) {
            setFuncIns(chartRef.current.getInstance());
        }
        setLoading(false);
    }, [session.deviceId, session.eventType, JSON.stringify(session.funcData.traces), session.maxTime, session.minTime]);
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
            height={'350px'}
            width={'calc(100vw - 80px)'}
            loading={loading}
            options={chartOptions}
            onEvents={
                {
                    datazoom: (params): void => {
                        const { startValue, endValue } = params.batch[0];
                        getFuncNewData(session, Math.floor(startValue), Math.ceil(endValue));
                    },
                    restore: (): void => {
                        getFuncNewData(session);
                    },
                }
            } />
    );
});
export default MemoryFunctionCall;
