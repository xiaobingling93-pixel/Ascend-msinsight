/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import * as React from 'react';
import { Graph } from '../entity/memory';
import { useResizeEventDependency, binarySearch } from '../utils/memoryUtils';
import * as echarts from 'echarts';

interface IProps {
    graph: Graph;
    hAxisTitle?: string;
    vAxisTitle?: string;
    onSelectionChanged?: (start: number, end: number) => void;
    record?: any;
    isDark: boolean;
}

type T = string | undefined;
const _getOriginOption = (graphTitle: T, hAxisTitle: T, vAxisTitle: T, isDark: boolean): echarts.EChartsOption => {
    return {
        title: {
            text: graphTitle,
            textStyle: {
                fontSize: 16,
            },
        },
        tooltip: {
            trigger: 'axis',
            textStyle: {
                align: 'left',
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
        backgroundColor: isDark ? '#252526' : '#ffffff',
    };
};

const _handleOption = (option: echarts.EChartsOption, graph: Graph): echarts.EChartsOption => {
    const allocatedTitle = [ graph.columns[0], graph.columns[1] ];
    const reservedTitle = [ graph.columns[0], graph.columns[2] ];
    const lineSerie: echarts.SeriesOption = {
        type: 'line',
        name: 'Operator Allocated',
        emphasis: {
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
    option = {
        ...option,
        dataset: [
            {
                source: [ allocatedTitle, ...graph.rows.allocated ],
            },
            {
                source: [ reservedTitle, ...graph.rows.reserved ],
            },
        ],
        series: [
            lineSerie,
            { ...lineSerie, name: 'Operator Reserved', datasetIndex: 1 },
        ],
    };
    if (graph.columns.length === 4) {
        const appTitle = [ graph.columns[0], graph.columns[3] ];
        (option.dataset as echarts.DatasetComponentOption[]).push({
            source: [ appTitle, ...graph.rows.app ],
        });
        (option.series as echarts.SeriesOption[]).push({
            ...lineSerie, name: 'APP Reserved', datasetIndex: 2,
        });
    }

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

const _handleEvents = (chartObj: echarts.ECharts | undefined, props: IProps, selectedPoints: React.MutableRefObject<number[]>): void => {
    const { graph, record } = props;
    const compareFun = (key: number, mid: number[]): number => key - parseFloat(mid[0].toFixed(2));
    if (chartObj) {
        if (record !== undefined) {
            const startId = binarySearch(graph.rows.allocated, record.col2, compareFun);
            const endId = binarySearch(graph.rows.allocated, record.col3, compareFun);
            const selection = [];
            startId >= 0 && selection.push(startId);
            endId >= 0 && selection.push(endId);
            chartObj.dispatchAction({
                type: 'downplay',
                seriesName: 'Allocated',
                dataIndex: selectedPoints.current,
            });
            chartObj.dispatchAction({
                type: 'highlight',
                seriesName: 'Allocated',
                dataIndex: selection,
            });
            selectedPoints.current = selection;
        } else {
            chartObj.dispatchAction({
                type: 'downplay',
                seriesName: 'Allocated',
                dataIndex: selectedPoints.current,
            });
            selectedPoints.current = [];
        }
    }
};

export const LineChart: React.FC<IProps> = (props) => {
    const { graph, record, isDark } = props;
    const graphRef = React.useRef<HTMLDivElement>(null);
    const [resizeEventDependency] = useResizeEventDependency();
    const [ chartObj, setChartObj ] = React.useState<echarts.ECharts | undefined>();
    const selectedPoints = React.useRef<number[]>([]);

    React.useLayoutEffect(() => {
        const element = graphRef.current;
        if (!element) {
            return;
        }
        element.oncontextmenu = () => { return false; };

        const myChart = echarts.init(element, isDark ? 'dark' : 'customed');
        _showGraph(myChart, selectedPoints, props, isDark);

        setChartObj(myChart);
        return () => {
            myChart.clear();
        };
    }, [ graph, resizeEventDependency, isDark ]);

    React.useEffect(() => {
        _handleEvents(chartObj, props, selectedPoints);
    }, [ graph, record, chartObj ]);

    return (
        <div ref={graphRef} style={{ height: '400px' }}></div>
    );
};
