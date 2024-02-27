/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import * as React from 'react';
import { Graph, OperatorDetail } from '../entity/memory';
import { useResizeEventDependency, binarySearch } from '../utils/memoryUtils';
import * as echarts from 'echarts';
import { chartCharacter } from './Common';

interface IProps {
    graph: Graph;
    hAxisTitle?: string;
    vAxisTitle?: string;
    onSelectionChanged?: (start: number, end: number) => void;
    record?: OperatorDetail;
    isDark: boolean;
    isWakeup: boolean;
}

type T = string | undefined;
const _getOriginOption = (graphTitle: T, hAxisTitle: T, vAxisTitle: T, isDark: boolean): echarts.EChartsOption => {
    return {
        title: {
            text: '',
        },
        tooltip: {
            trigger: 'axis',
            formatter: function (params: any) {
                let res = `${params?.[0]?.name} <br/>`;
                for (const item of params) {
                    if (!isNaN(Number(item?.value?.[item?.encode?.y?.[0]]))) {
                        res += `<span style="background: ${item.color};
                        height:10px;
                        width: 10px;
                        border-radius: 50%;
                        display: inline-block;
                        margin-right:10px;">
                        </span>
                        ${item.seriesName}: ${item?.value?.[item?.encode?.y?.[0]]}<br/>`;
                    }
                }
                return res;
            },
        },
        legend: {
            type: 'scroll',
            bottom: 0,
        },
        xAxis: {
            type: 'category',
            boundaryGap: false,
            name: hAxisTitle,
        },
        yAxis: {
            type: 'value',
            name: vAxisTitle,
            scale: true,
        },
        toolbox: {
            feature: {
                dataZoom: {
                    yAxisIndex: 'none',
                },
                restore: {},
            },
        },
        backgroundColor: isDark ? '#1e1e1e' : '#ffffff',
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
    option = {
        ...option,
        dataset:
        {
            source: [graph.columns, ...graph.rows],
        },
        series: Array(graph.columns.length - 1).fill(lineSerie),
    };
    return option;
};

const _showGraph = (myChart: echarts.ECharts, selectedPoints: React.MutableRefObject<number[]>,
    props: IProps, isDark: boolean): void => {
    const { graph, hAxisTitle, vAxisTitle, onSelectionChanged } = props;

    let option = _getOriginOption(graph.title, hAxisTitle, vAxisTitle, isDark);
    option = _handleOption(option, graph);

    myChart.setOption(option, true);

    myChart.dispatchAction({
        type: 'takeGlobalCursor',
        key: 'dataZoomSelect',
        dataZoomSelectActive: true,
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

const _handleEvents = (chartObj: echarts.ECharts | undefined, props: IProps,
    selectedPoints: React.MutableRefObject<number[]>, graph: Graph): void => {
    const { record } = props;
    const compareFun = (key: number, mid: Array<number | string>): number => key - parseFloat(mid[0] as string);
    if (chartObj) {
        if (record !== undefined) {
            const startId = binarySearch(graph.rows, record.allocationTime, compareFun);
            const endId = binarySearch(graph.rows, record.releaseTime, compareFun);
            const selection = [];
            startId >= 0 && selection.push(startId);
            endId >= 0 && selection.push(endId);
            chartObj.dispatchAction({
                type: 'downplay',
                seriesName: 'Operators Allocated',
                dataIndex: selectedPoints.current,
            });
            chartObj.dispatchAction({
                type: 'highlight',
                seriesName: 'Operators Allocated',
                dataIndex: selection,
            });
            selectedPoints.current = selection;
        } else {
            chartObj.dispatchAction({
                type: 'downplay',
                seriesName: 'Operators Allocated',
                dataIndex: selectedPoints.current,
            });
            selectedPoints.current = [];
        }
    }
};

export const LineChart: React.FC<IProps> = (props) => {
    const { graph, record, isDark, isWakeup, onSelectionChanged } = props;
    const graphRef = React.useRef<HTMLDivElement>(null);
    const [resizeEventDependency] = useResizeEventDependency();
    const [chartObj, setChartObj] = React.useState<echarts.ECharts | undefined>();
    const selectedPoints = React.useRef<number[]>([]);

    React.useLayoutEffect(() => {
        const element = graphRef.current;
        if (!element) {
            return;
        }
        element.oncontextmenu = () => { return false; };

        const myChart = echarts.init(element, isDark ? 'dark' : 'customed');
        onSelectionChanged?.(0, -1);
        _showGraph(myChart, selectedPoints, props, isDark);

        setChartObj(myChart);
        return () => {
            myChart.dispose();
        };
    }, [graph, resizeEventDependency, isDark, isWakeup]);

    React.useEffect(() => {
        _handleEvents(chartObj, props, selectedPoints, graph);
    }, [graph, record, chartObj]);

    return (
        <div>
            {graph.title?.length !== 0
                ? <div style={{ fontSize: 14, fontWeight: 'bold' } }>
                    {graph.title}{chartCharacter}
                </div>
                : <div style={{ fontSize: 14, fontWeight: 'bold' } }>
                    {graph.title}
                </div>
            }
            <div ref={graphRef} style={{ height: '400px' }}></div>
        </div>
    );
};
