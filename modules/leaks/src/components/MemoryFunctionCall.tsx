/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useEffect, useState, useRef } from 'react';
import * as echarts from 'echarts';
import { safeStr } from 'ascend-utils';
import { MIChart } from 'ascend-components';
import type { ChartsHandle } from 'ascend-components/MIChart';
import { type EChartsOption } from 'echarts';
import { Session } from '../entity/session';
import { observer } from 'mobx-react';
import { getFuncNewData } from './dataHandler';
import { chartResize } from '../utils/utils';
import { useTheme, type Theme } from '@emotion/react';
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
        right: 80,
        top: 20,
    };
};
const getRenderItem = (session: Session, theme: Theme, api: any): any => {
    const level = api.value(0);
    const start = api.coord([Math.max(api.value(1), session.minTime), level]);
    const end = api.coord([Math.min(api.value(2), session.maxTime), level]);
    const height = ((api.size([0, 1]) || [0, 20]) as number[])[1];
    const width = end[0] - start[0];
    const customRes = {
        type: 'rect',
        transition: ['shape'],
        shape: {
            x: start[0],
            y: start[1],
            width,
            height: height - 2,
        },
        emphasis: { style: { stroke: '#000' } },
        textContent: {
            type: 'text',
            style: {
                text: api.value(3),
                fill: '#000',
                overflow: 'truncate',
                width: width - 4,
                fontSize: 11,
            },
        },
        textConfig: {
            position: 'inside',
            inside: true,
            local: true,
        },
        style: {
            fill: api.visual('color'),
            stroke: '',
            lineWidth: 0,
        },
    };
    if (session.searchFunc.includes(api.value(3))) {
        customRes.style.stroke = theme.mode === 'dark' ? '#fff' : '#000';
        customRes.style.lineWidth = 3;
    }
    return customRes;
};
const getSeries = (session: Session, theme: Theme): any => {
    return [
        {
            type: 'custom',
            renderItem: (params: any, api: any): any => getRenderItem(session, theme, api),
            encode: {
                x: [0, 1, 2],
                y: 0,
            },
            data: transData(session.funcData.traces),
            clip: true,
        },
    ];
};
const getTooltip = (session: Session): echarts.TooltipComponentOption => {
    return {
        trigger: 'item',
        formatter: function (params: any): string {
            const info = session.funcData.traces[params.dataIndex];
            if (!info) {
                return '';
            }
            return safeStr(info.func);
        },
        textStyle: {
            fontSize: 12,
        },
    };
};
const getOptions = (session: Session, theme: Theme): EChartsOption => {
    return {
        color: ['#5470c6', '#91cc75', '#fac858', '#ee6666', '#73c0de', '#3ba272', '#fc8452', '#9a60b4', '#ea7ccc'],
        gradientColor: ['#f6efa6', '#d88273', '#bf444c'],
        xAxis: {
            axisTick: {
                show: false,
            },
            min: session.minTime,
            max: session.maxTime,
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
        tooltip: getTooltip(session),
        series: getSeries(session, theme),
        grid: {
            left: '8%',
            right: '4%',
        },
    };
};

const MemoryFunctionCall = observer(({ session, setFuncIns }: {
    session: Session;
    setFuncIns: (value: echarts.ECharts | null) => void;
}): React.ReactElement => {
    const chartRef = useRef<ChartsHandle>(null);
    const [loading, setLoading] = useState(false);
    const [chartOptions, setChartOptions] = useState<EChartsOption>({});
    const { funcData, deviceId, eventType, threadId, maxTime, minTime, synStartTime, synEndTime, searchFunc } = session;
    const [startTime, setStartTime] = useState<number>(synStartTime);
    const [endTime, setEndTime] = useState<number>(synEndTime);
    const theme: Theme = useTheme();
    useEffect(() => {
        if (deviceId === '') return;
        setLoading(true);
        if (startTime !== synStartTime || endTime !== synEndTime) {
            setStartTime(synStartTime);
            setEndTime(synEndTime);
            getFuncNewData(session, synStartTime, synEndTime);
        } else {
            getFuncNewData(session);
        }
    }, [deviceId, eventType, threadId, synStartTime, synEndTime]);
    useEffect(() => {
        setChartOptions(getOptions(session, theme));
        if (chartRef.current !== null && chartRef.current !== undefined) {
            setFuncIns(chartRef.current.getInstance());
        }
        setLoading(false);
        chartResize(chartRef?.current?.getInstance());
    }, [deviceId, eventType, JSON.stringify(funcData.traces), maxTime, minTime]);
    useEffect(() => {
        setChartOptions(getOptions(session, theme));
    }, [JSON.stringify(searchFunc), theme.mode]);
    useEffect(() => {
        chartRef.current?.getInstance()?.dispatchAction({
            type: 'takeGlobalCursor',
            key: 'dataZoomSelect',
            dataZoomSelectActive: true,
        });
    }, [chartOptions, theme.mode]);
    return (
        <MIChart
            ref={chartRef}
            height="500px"
            width="calc(100vw - 80px)"
            loading={loading}
            options={chartOptions}
        />
    );
});
export default MemoryFunctionCall;
