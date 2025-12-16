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
import React, { useState, useEffect, useMemo, useRef } from 'react';
import * as echarts from 'echarts';
import * as d3 from 'd3';
import { useTranslation } from 'react-i18next';
import { MIChart } from '@insight/lib/components';
import type { ChartsHandle } from '@insight/lib';
import { safeStr } from '@insight/lib/utils';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import { useTheme } from '@emotion/react';
import type { TFunction } from 'i18next';
import { type EChartsOption } from 'echarts';
import type { BlockData } from '../utils/RequestUtils';
import { getBarNewData, getFuncNewData } from './dataHandler';
import { chartResize } from '../utils/utils';
import { Session } from '../entity/session';
import { ContextMenu, type MenuItemModel } from './ContextMenu';
import { Line } from './LineHandler';
const colorScale = d3.scaleOrdinal(d3.schemeCategory10);
interface InitParam {
    session: Session;
    blockData: BlockData;
    lineSource: number[][];
    source: number[][];
    t: TFunction;
};
const getSource = (session: Session): number[][] => {
    const blockSource: number[][] = [];
    session.blockData.blocks.forEach((block: any, index: number) => {
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
};
const getXAxis = (session: Session): echarts.XAXisComponentOption => {
    return {
        type: 'value',
        min: session.minTime,
        max: session.maxTime,
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
            formatter: (value: number): string => `${(value / 1000000000).toFixed(3)}s`,
        },
        splitLine: {
            show: false,
        },
    };
};
const getYAxis = (t: TFunction): echarts.YAXisComponentOption => {
    return {
        type: 'value',
        axisLine: {
            show: true,
        },
        axisTick: {
            show: false,
        },
        axisLabel: {
            formatter: (value: number): string => (value / 1024 / 1024).toFixed(3),
        },
        splitLine: {
            show: false,
        },
        scale: true,
        axisPointer: {
            show: false,
        },
        name: t('MemoryUsage'),
    };
};
const getSeries = (t: TFunction, source: InitParam['source'], lineSource: InitParam['lineSource']): any => {
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
                    shape: { points },
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
const getLegend = (t: TFunction, session: Session): echarts.LegendComponentOption => {
    return {
        data: [t('MemoryAllocations'), t('MemoryBlocks')],
        selected: session.legendSelect,
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
            return safeStr(`Addr: ${info.addr} Size: ${(info.size / 1024 / 1024).toFixed(3)}MB Life: ${((info.endTimestamp - info.startTimestamp) / 1000000000).toFixed(3)}s`);
        },
    };
};
const getOptions = ({ session, blockData, lineSource, source, t }: InitParam): EChartsOption => {
    return {
        tooltip: getTooltip(blockData),
        xAxis: getXAxis(session),
        yAxis: getYAxis(t),
        legend: getLegend(t, session),
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
            right: 80,
            top: 20,
        },
        axisPointer: {
            show: true,
        },
        grid: {
            left: 80,
            right: 60,
        },
    };
};
const handleDblclick = (barIns: echarts.ECharts | null | undefined, session: Session): void => {
    if (barIns === undefined || barIns === null) return;
    barIns?.off('dblclick');
    barIns?.on('dblclick', (params: any): void => {
        const info = session.blockData.blocks[params.dataIndex];
        if (!info) {
            return;
        }
        runInAction(() => {
            if (session.threadId !== info.threadId) {
                session.searchFunc = [];
            } else {
                const funcSet = new Set(session.searchFunc);
                session.searchFunc = funcSet.size ? session.funcOptions.filter((item: any) => funcSet.has(item.value)).map((i: any) => i.value) : [];
            }
            session.threadFlag = true;
            session.threadId = info.threadId;
            getBarNewData(session, info.startTimestamp, info.endTimestamp);
            getFuncNewData(session, info.startTimestamp, info.endTimestamp);
        });
    });
};
const handleContextMenu = (barIns: echarts.ECharts | null | undefined, session: Session): void => {
    if (barIns === undefined || barIns === null) return;
    barIns?.off('contextmenu');
    const contextmenuCb = (params: any): void => {
        const index = params?.dataIndex;
        const firstAccessTimestamp = session.blockData.blocks[index].firstAccessTimestamp;
        const lastAccessTimestamp = session.blockData.blocks[index].lastAccessTimestamp;
        if (firstAccessTimestamp === -1 && lastAccessTimestamp === -1) {
            const event = params.event.event;
            runInAction(() => {
                session.allowMark = false;
                session.contextMenu.xPos = event.clientX;
                session.contextMenu.yPos = event.clientY;
                session.contextMenu.visible = true;
            });
        } else {
            const startPoint = barIns?.convertToPixel('xAxis', firstAccessTimestamp);
            const endPoint = barIns?.convertToPixel('xAxis', lastAccessTimestamp);
            const event = params.event.event;
            runInAction(() => {
                session.allowMark = true;
                session.firstLastStamps = {
                    first: startPoint + 24,
                    last: endPoint + 24,
                };
                session.contextMenu.xPos = event.clientX;
                session.contextMenu.yPos = event.clientY;
                session.contextMenu.visible = true;
            });
        }
    };
    barIns?.on('contextmenu', contextmenuCb);
};
const handleZrClick = (barIns: echarts.ECharts | null | undefined, session: Session, chartRef: React.RefObject<ChartsHandle>, t: TFunction): void => {
    if (barIns === undefined || barIns === null) return;
    barIns?.getZr()?.off('click');
    barIns?.getZr()?.on('click', (params) => {
        if (params.target?.constructor?.name === 'ECPolyline' || params.target?.constructor?.name === 'Polygon' ||
            params.target?.constructor?.name === undefined) {
            const pointInPixel = [params.offsetX, params.offsetY];
            const pointInGrid = chartRef.current?.getInstance()?.convertFromPixel({ seriesIndex: 0 }, pointInPixel);
            const xValue = pointInGrid?.[0];
            if (xValue === undefined || xValue < session.minTime || xValue > session.maxTime) return;
            runInAction(() => {
                session.memoryStamp = Number(xValue?.toFixed(0));
            });
        }
    });
};
const restoreLine = (session: Session): void => {
    runInAction(() => {
        session.markLineshow = 'none';
        session.firstOffset = 0;
        session.lastOffset = 0;
        session.firstLastStamps = { first: 0, last: 0 };
    });
};
const MemoryBarChart = observer(({ session, setBarIns }: { session: Session; setBarIns: (value: echarts.ECharts | null) => void }): React.ReactElement => {
    const { t } = useTranslation('leaks');
    const chartRef = useRef<ChartsHandle>(null);
    const [loading, setLoading] = useState(false);
    const [chartOptions, setChartOptions] = useState<EChartsOption>({});
    const { blockData, allocationData, deviceId, eventType, threadId, threadFlag, markLineshow, firstOffset, lastOffset, firstLastStamps } = session;
    const lineSource = allocationData.allocations.map((line: any) => [line.timestamp, line.totalSize]);
    const source: number[][] = useMemo(() => getSource(session), [JSON.stringify(blockData.blocks)]);
    const initParam: InitParam = { session, blockData, lineSource, source, t };
    const theme = useTheme();
    const getMenuItems = (): MenuItemModel[] => {
        const allMenuItems: MenuItemModel[] = [
            {
                label: t('markFirstLastTime'),
                key: 'markFirstLastTime',
                action: (): void => {
                    runInAction(() => {
                        session.markLineshow = 'block';
                        session.firstOffset = firstLastStamps.first;
                        session.lastOffset = firstLastStamps.last;
                    });
                },
                visible: true,
                disabled: !session.allowMark,
            },
            {
                label: t('cancelMarkTime'),
                key: 'cancelMarkTime',
                action: (): void => { runInAction(() => { session.markLineshow = 'none'; }); },
                visible: session.markLineshow === 'block',
            },
        ];
        return allMenuItems.filter(menuItem => menuItem.visible !== false);
    };
    useEffect(() => {
        if (deviceId === '' || threadFlag) return;
        setLoading(true);
        getBarNewData(session);
    }, [deviceId, eventType, threadId]);
    useEffect(() => {
        const param: EChartsOption = getOptions(initParam);
        setChartOptions(param);
        restoreLine(session);
        const barIns: echarts.ECharts | null | undefined = chartRef.current?.getInstance();
        if (barIns !== null && barIns !== undefined) {
            setBarIns(barIns);
        }
        setLoading(false);
        handleDblclick(barIns, session);
        handleContextMenu(barIns, session);
        handleZrClick(barIns, session, chartRef, t);
        chartResize(barIns);
    }, [threadId, JSON.stringify(blockData), JSON.stringify(allocationData), session.maxTime, session.minTime, t]);
    useEffect(() => {
        chartRef.current?.getInstance()?.dispatchAction({
            type: 'takeGlobalCursor',
            key: 'dataZoomSelect',
            dataZoomSelectActive: true,
        });
    }, [chartOptions, theme]);
    return (
        <>
            <MIChart
                ref={chartRef}
                width="calc(100vw - 120px)"
                height="500px"
                loading={loading}
                options={chartOptions}
            />
            <Line id="firstLine" lineShow={markLineshow} offset={firstOffset} color="green" />
            <Line id="lastLine" lineShow={markLineshow} offset={lastOffset} color="green" />
            <ContextMenu session={session} menuItems={getMenuItems()} />
        </>
    );
});
export default MemoryBarChart;
