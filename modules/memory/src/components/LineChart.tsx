/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import * as React from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import type { Graph } from '../entity/memory';
import { binarySearch, useResizeEventDependency } from '../utils/memoryUtils';
import * as echarts from 'echarts';
import { convertTime, useChartCharacter } from './Common';
import styled from '@emotion/styled';
import { chartColors, getDefaultChartOptions, safeStr } from 'ascend-utils';

const ChartDesc = styled.div`
    color: ${(props): string => props.theme.textColor};
    margin-bottom: 24px;
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

const _getLegendData = (data: string[]): string[] => {
    const tempData = [...data];
    // 去除表示横轴的column列
    tempData.shift();
    if (tempData.length < 2) {
        return tempData;
    }
    for (let i = 1; i < tempData.length; i++) {
        // 遍历到基线卡即可根据上一个图例是否包含比对信息而决定是否需要换行
        if ((tempData[i].endsWith('Baseline') || tempData[i].startsWith('基线'))) {
            if ((tempData[i - 1].endsWith('Comparison') || tempData[i - 1].startsWith('比对'))) {
                tempData.splice(i, 0, '');
            }
            return tempData;
        }
    }
    return tempData;
};

const _getOriginOption = (hAxisTitle: string, vAxisTitle: string, isDark: boolean, isStatic: boolean, graph: Graph): echarts.EChartsOption => {
    const legendDatas = _getLegendData(graph.columns);
    return {
        textStyle: getDefaultChartOptions().textStyle,
        title: { text: '' },
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
                        margin-right: 10px;"></span>
                        ${safeStr(item.seriesName)}: ${safeStr(item?.value?.[item?.encode?.y?.[0]])}<br/>`;
                    }
                }
                return res;
            },
            ...getDefaultChartOptions(isDark).tooltip,
        },
        legend: { itemGap: 20, data: legendDatas },
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
    props: IProps, isDark: boolean, isStatic: boolean): void => {
    const { graph, hAxisTitle, vAxisTitle, onSelectionChanged } = props;

    let option = _getOriginOption(hAxisTitle, vAxisTitle, isDark, isStatic, graph);
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
    const { t, i18n } = useTranslation('memory');
    const locale = i18n.language?.slice(0, 2);

    React.useLayoutEffect(() => {
        const element = graphRef.current;
        if (!element) {
            return () => {};
        }
        element.oncontextmenu = (): boolean => { return false; };

        const myChart = echarts.init(element, isDark ? 'dark' : 'customed', { locale });
        onSelectionChanged?.(0, -1);
        _showGraph(myChart, selectedPoints, props, isDark, isStatic);

        setChartObj(myChart);
        return () => {
            myChart.dispose();
        };
    }, [graph, isDark, i18n]);

    React.useEffect(() => {
        if (!graphRef.current) {
            return;
        }
        echarts.getInstanceByDom(graphRef.current)?.resize();
    }, [resizeEventDependency]);

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
