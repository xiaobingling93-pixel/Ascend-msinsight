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
import * as React from 'react';
import { useTranslation } from 'react-i18next';
import type { Graph } from '../entity/curve';
import { useResizeEventDependency } from '../utils/curveUtils';
import * as echarts from 'echarts';
import { chartColors, getDefaultChartOptions, getLegendStyle, safeStr } from '@insight/lib/utils';
import { type Theme, useTheme } from '@emotion/react';

// 最大不分页的折线图图例数量，超过该数量图例分页展示
const MAX_PLAIN_LEGENDS_COUNT = 9;

interface IProps {
    graph: Graph;
    hAxisTitle: string;
    vAxisTitle: string;
    onSelectionChanged?: (start: number, end: number) => void;
    record?: any;
    isDark: boolean;
}

const _getLegendData = (data: string[]): string[] => {
    const tempData = [...data];
    // 去除表示横轴的column列
    tempData.shift();
    return tempData;
};

const format = (params: any): string => {
    let res = `${safeStr(params?.[0]?.name)} <br/>`;
    for (const item of params) {
        if (!isNaN(Number(item?.value?.[item?.encode?.y?.[0]]))) {
            res += `<span style="background: ${item.color};
                        height: 10px;
                        width: 10px;
                        border-radius: 50%;
                        display: inline-block;
                        margin-right: 10px;"></span>
                        ${safeStr(item.seriesName)}: ${safeStr(item?.value?.[item?.encode?.y?.[0]])}<br/>`;
        }
    }
    return res;
};

const _getOriginOption = (props: IProps, theme: Theme): echarts.EChartsOption => {
    const { isDark, hAxisTitle, vAxisTitle } = props;
    const legendDatas = _getLegendData(props.graph.columns);
    return {
        textStyle: getDefaultChartOptions().textStyle,
        title: { text: '' },
        tooltip: {
            trigger: 'axis',
            formatter: (params: any): string => {
                return format(params);
            },
            ...getDefaultChartOptions(isDark).tooltip,
        },
        legend: {
            itemGap: 20,
            data: legendDatas,
            type: legendDatas.length > MAX_PLAIN_LEGENDS_COUNT ? 'scroll' : 'plain',
            ...getLegendStyle(theme),
        },
        grid: { left: '100', right: '100', bottom: 40 },
        xAxis: { type: 'category', boundaryGap: false, name: hAxisTitle },
        yAxis: { type: 'value', name: vAxisTitle, scale: true },
        toolbox: {
            feature: {
                dataZoom: {
                    yAxisIndex: 'none',
                    emphasis: { iconStyle: { textPosition: 'top' } },
                },
                restore: {
                    emphasis: { iconStyle: { textPosition: 'top' } },
                },
            },
            top: 20,
            right: 10,
        },
        backgroundColor: 'transparent',
    };
};

const _handleOption = (option: echarts.EChartsOption, graph: Graph): echarts.EChartsOption => {
    const lineSerie: echarts.SeriesOption = {
        type: 'line',
        connectNulls: true,
        emphasis: {
            label: {
                show: true,
            },
            itemStyle: {
                borderWidth: 5,
                shadowBlur: 5,
                shadowColor: '#ffffff',
            },
        },
        select: {
            itemStyle: {
                borderWidth: 5,
                shadowBlur: 5,
            },
        },
    };
    const newOption = {
        ...option,
        color: chartColors,
        dataset:
        {
            source: [graph.columns, ...graph.rows],
        },
        series: Array(graph.columns.length - 1).fill(lineSerie),
    };
    return newOption;
};

const _showGraph = (myChart: echarts.ECharts, selectedPoints: React.MutableRefObject<number[]>,
    props: IProps, theme: Theme): void => {
    const { graph, onSelectionChanged } = props;

    let option = _getOriginOption(props, theme);
    option = _handleOption(option, graph);
    // 数据量大时，切换主题时setOption会阻塞整体界面主题切换，使用 requestAnimationFrame 优化
    requestAnimationFrame(() => {
        myChart.setOption(option, { notMerge: true, lazyUpdate: true });
        myChart.dispatchAction({
            type: 'takeGlobalCursor',
            key: 'dataZoomSelect',
            dataZoomSelectActive: true,
        });
    });

    myChart.on('dataZoom', (param: any) => {
        onSelectionChanged?.(param.batch[0].startValue, param.batch[0].endValue);
    });

    myChart.on('restore', () => {
        // Set startId greater than endId to query all memory events.
        onSelectionChanged?.(0, -1);
    });

    myChart.on('click', (param) => {
        myChart.dispatchAction({
            type: 'unselect',
            seriesId: param.seriesId,
            dataIndex: selectedPoints.current,
        });
        myChart.dispatchAction({
            type: 'select',
            seriesId: param.seriesId,
            dataIndex: param.dataIndex,
        });

        selectedPoints.current = [param.dataIndex];
    });

    myChart.getZr().on('contextmenu', () => {
        myChart.dispatchAction({
            type: 'restore',
        });
        myChart.dispatchAction({
            type: 'takeGlobalCursor',
            key: 'dataZoomSelect',
            dataZoomSelectActive: true,
        });
    });
};

export const LineChart: React.FC<IProps> = (props) => {
    const { graph, isDark, onSelectionChanged } = props;
    const graphRef = React.useRef<HTMLDivElement>(null);
    const [resizeEventDependency] = useResizeEventDependency();
    const selectedPoints = React.useRef<number[]>([]);
    const { i18n } = useTranslation('statistic');
    const locale = i18n.language?.slice(0, 2);
    const theme = useTheme();
    React.useEffect(() => {
        const element = graphRef.current;
        if (!element) {
            return () => {};
        }
        element.oncontextmenu = (): boolean => { return false; };
        const myChart = echarts.init(element, isDark ? 'dark' : 'customed', { locale });
        onSelectionChanged?.(0, -1);
        _showGraph(myChart, selectedPoints, props, theme);

        return () => {
            myChart.dispose();
        };
    }, [graph, isDark]);

    React.useEffect(() => {
        if (!graphRef.current) {
            return;
        }
        echarts.getInstanceByDom(graphRef.current)?.resize();
    }, [resizeEventDependency]);
    return (
        <div>
            <div ref={graphRef} style={{ width: 'calc(100vw - 80px)', height: '400px' }}></div>
        </div>
    );
};
