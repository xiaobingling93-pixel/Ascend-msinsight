/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import * as React from 'react';
import { Graph, OperatorDetail } from '../entity/memory';
import { useResizeEventDependency, binarySearch } from '../utils/memoryUtils';
import * as echarts from 'echarts';

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
            text: graphTitle,
            textStyle: { fontSize: 16 },
        },
        tooltip: {
            trigger: 'axis',
            formatter: function (params: any) {
                let res = `${params[0].name} <br/>`;
                for (const item of params) {
                    if (typeof item.value[item.encode.y[0]] === 'number') {
                        res += `<span style="background: ${item.color}; 
                        height:10px; 
                        width: 10px; 
                        border-radius: 50%;
                        display: inline-block;
                        margin-right:10px;">
                        </span> 
                        ${item.seriesName}: ${item.value[item.encode.y[0]]}<br/>`;
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

const _handleOption = (option: echarts.EChartsOption, graph: Graph, dataRowsRef: React.MutableRefObject<number[][]>): echarts.EChartsOption => {
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
        datasetIndex: 0,
    };
    const allocatedRows = graph.rows.allocated.map(item => {
        return [ parseFloat(item[0].toFixed(2)), item[1] ];
    });
    const reservedRows = graph.rows.reserved.map(item => {
        return [ parseFloat(item[0].toFixed(2)), null, item[1] ];
    });
    let finalRows = allocatedRows.concat(reservedRows as number[][]);
    if (graph.columns.length === 4) {
        const appRows = graph.rows.app.map(item => {
            return [ parseFloat(item[0].toFixed(2)), null, null, item[1] ];
        });
        finalRows = finalRows.concat(appRows as number[][]).sort((a, b) => {
            return a[0] - b[0];
        });
    }
    dataRowsRef.current = finalRows;
    option = {
        ...option,
        dataset:
        {
            source: [ graph.columns, ...finalRows ],
        },
        series: Array(graph.columns.length - 1).fill(lineSerie),
    };
    return option;
};

const _showGraph = (myChart: echarts.ECharts, selectedPoints: React.MutableRefObject<number[]>,
    props: IProps, isDark: boolean, dataRowsRef: React.MutableRefObject<number[][]>): void => {
    const { graph, hAxisTitle, vAxisTitle, onSelectionChanged } = props;

    let option = _getOriginOption(graph.title, hAxisTitle, vAxisTitle, isDark);
    option = _handleOption(option, graph, dataRowsRef);

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
    selectedPoints: React.MutableRefObject<number[]>, dataRowsRef: React.MutableRefObject<number[][]>): void => {
    const { graph, record } = props;
    const compareFun = (key: number, mid: number[]): number => key - parseFloat(mid[0].toFixed(2));
    if (chartObj) {
        if (record !== undefined) {
            const startId = binarySearch(dataRowsRef.current, record.allocationTime, compareFun);
            const endId = binarySearch(dataRowsRef.current, record.releaseTime, compareFun);
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
    const { graph, record, isDark, isWakeup } = props;
    const graphRef = React.useRef<HTMLDivElement>(null);
    const [resizeEventDependency] = useResizeEventDependency();
    const [ chartObj, setChartObj ] = React.useState<echarts.ECharts | undefined>();
    const dataRowsRef = React.useRef<number[][]>([]);
    const selectedPoints = React.useRef<number[]>([]);

    React.useLayoutEffect(() => {
        const element = graphRef.current;
        if (!element) {
            return;
        }
        element.oncontextmenu = () => { return false; };

        const myChart = echarts.init(element, isDark ? 'dark' : 'customed');
        _showGraph(myChart, selectedPoints, props, isDark, dataRowsRef);

        setChartObj(myChart);
        return () => {
            myChart.dispose();
        };
    }, [ graph, resizeEventDependency, isDark, isWakeup ]);

    React.useEffect(() => {
        _handleEvents(chartObj, props, selectedPoints, dataRowsRef);
    }, [ graph, record, chartObj ]);

    return (
        <div ref={graphRef} style={{ height: '400px' }}></div>
    );
};
