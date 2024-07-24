/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import * as React from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import type { Graph } from '../entity/memory';
import { binarySearch, useResizeEventDependency } from '../utils/memoryUtils';
import * as echarts from 'echarts';
import { convertTime, safeStr, useChartCharacter } from './Common';
import styled from '@emotion/styled';

const ChartDesc = styled.div`
    color: ${(props): string => props.theme.textColorTertiary}
`;

interface IProps {
    graph: Graph;
    hAxisTitle: string;
    vAxisTitle: string;
    onSelectionChanged?: (start: number, end: number) => void;
    record?: any;
    isDark: boolean;
    isStatic: boolean;
}

const _getOriginOption = (hAxisTitle: string, vAxisTitle: string, isDark: boolean, isStatic: boolean): echarts.EChartsOption => {
    return {
        title: {
            text: '',
        },
        tooltip: {
            trigger: 'axis',
            formatter: (params: any): string => {
                let res = `${isStatic ? safeStr(params?.[0]?.name) : convertTime(params?.[0]?.name)} <br/>`;
                for (const item of params) {
                    if (!isNaN(Number(item?.value?.[item?.encode?.y?.[0]]))) {
                        res += `<span style="background: ${item.color};
                        height: 10px;
                        width: 10px;
                        border-radius: 50%;
                        display: inline-block;
                        margin-right: 10px;">
                        </span>
                        ${safeStr(item.seriesName)}: ${safeStr(item?.value?.[item?.encode?.y?.[0]])}<br/>`;
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
        color: [
            '#5470c6', '#91cc75', '#fac858', '#ee6666', '#73c0de',
            '#3ba272', '#865c4b', '#9a60b4', '#ea7ccc', '#6f83a5',
            '#c86290', '#6aad92', '#d28b5d', '#7a929e', '#b66579',
            '#69b0ac', '#8d6e6d', '#9db3aa', '#e89e99', '#7397b9',
            '#cf8191', '#809abc', '#c2a26b', '#75adb6', '#ca87b0',
            '#78a2bb', '#cc2adf', '#86a087', '#d8a895', '#8c7e6d',
            '#b3b9c1', '#d39974', '#728eaa', '#d5aa98', '#88a6bc',
            '#db817a', '#6487a8', '#cd918d', '#a2bce6', '#d19c7d',
            '#6e97ab', '#c09c8d', '#41f059', '#d29d88', '#5e8c8e',
            '#c2b5a5', '#07276a', '#f4f0ee', '#7e737f', '#d1a082',
        ],
        dataset:
        {
            source: [graph.columns, ...graph.rows],
        },
        series: Array(graph.columns.length - 1).fill(lineSerie),
    };
    return newOption;
};

const _showGraph = (myChart: echarts.ECharts, selectedPoints: React.MutableRefObject<number[]>,
    props: IProps, isDark: boolean, isStatic: boolean): void => {
    const { graph, hAxisTitle, vAxisTitle, onSelectionChanged } = props;

    let option = _getOriginOption(hAxisTitle, vAxisTitle, isDark, isStatic);
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
    selectedPoints: React.MutableRefObject<number[]>, graph: Graph, t: TFunction): void => {
    const { record } = props;
    const compareFun = (key: number, mid: Array<number | string>): number => key - parseFloat(mid[0] as string);
    if (chartObj) {
        if (record !== undefined) {
            const startId = binarySearch(graph.rows, Number(record?.allocationTime ?? record?.nodeIndexStart), compareFun);
            const endId = binarySearch(graph.rows, Number(record?.releaseTime ?? record?.nodeIndexEnd), compareFun);
            const selection = [];
            if (startId >= 0) {
                selection.push(startId);
            }
            if (endId >= 0) {
                selection.push(endId);
            }
            chartObj.dispatchAction({
                type: 'downplay',
                seriesName: t('Operators Allocated'),
                dataIndex: selectedPoints.current,
            });
            chartObj.dispatchAction({
                type: 'highlight',
                seriesName: t('Operators Allocated'),
                dataIndex: selection,
            });
            selectedPoints.current = selection;
        } else {
            chartObj.dispatchAction({
                type: 'downplay',
                seriesName: t('Operators Allocated'),
                dataIndex: selectedPoints.current,
            });
            selectedPoints.current = [];
        }
    }
};

const useTitle = (title: string): string => {
    const { t } = useTranslation('memory', { keyPrefix: 'searchCriteria' });
    const regex = /Peak Memory Usage|Operator Activated|Operator Allocated|Operator Reserved|APP Reserved/g;
    const translatedMessage = title.replace(regex, match => t(match));
    return translatedMessage;
};
export const LineChart: React.FC<IProps> = (props) => {
    const { graph, record, isDark, isStatic, onSelectionChanged } = props;
    const graphRef = React.useRef<HTMLDivElement>(null);
    const [resizeEventDependency] = useResizeEventDependency();
    const [chartObj, setChartObj] = React.useState<echarts.ECharts | undefined>();
    const selectedPoints = React.useRef<number[]>([]);
    const chartCharacter = useChartCharacter();
    const title = useTitle(graph.title ?? '');
    const { t } = useTranslation('memory');

    React.useLayoutEffect(() => {
        const element = graphRef.current;
        if (!element) {
            return () => {};
        }
        element.oncontextmenu = (): boolean => { return false; };

        const myChart = echarts.init(element, isDark ? 'dark' : 'customed');
        onSelectionChanged?.(0, -1);
        _showGraph(myChart, selectedPoints, props, isDark, isStatic);

        setChartObj(myChart);
        return () => {
            myChart.dispose();
        };
    }, [graph, resizeEventDependency, isDark]);

    React.useEffect(() => {
        _handleEvents(chartObj, props, selectedPoints, graph, t);
    }, [graph, record, chartObj]);

    return (
        <div>
            {graph.title !== undefined && graph.title?.length !== 0
                ? <ChartDesc>{title}{chartCharacter}</ChartDesc>
                : null
            }
            <div ref={graphRef} style={{ width: 'calc(100vw - 80px)', height: '400px' }}></div>
        </div>
    );
};
