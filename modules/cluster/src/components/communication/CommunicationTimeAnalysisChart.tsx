/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import * as echarts from 'echarts';

import type { Session } from '../../entity/session';
import React, { useEffect, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react-lite';
import { getBaselineName, getCompareName, Loading } from '../Common';
import { colorPalette, hashToNumber } from '../../utils/colorUtil';
import { Dropdown } from '@insight/lib/components';
import { type MenuProps, message, Spin } from 'antd';
import connector from '../../connection';
import i18n from '@insight/lib/i18n';
import { themeInstance } from '@insight/lib/theme';
import { type Theme } from '@emotion/react';
import { disposeAdaptiveEchart, getAdaptiveEchart, getDefaultChartOptions, safeStr } from '@insight/lib/utils';
import { ChartZoomData, ClickOperatorItem, CompareData, type ErrorInfo, FormatterParams } from '../../utils/interface';
import { queryTimelineUnitKernelDetail } from '../../utils/RequestUtils';
import { useEventBus } from '../../utils/eventBus';
import type { ECharts, InsideDataZoomComponentOption } from 'echarts';

// 定义点击慢操作排名时的回调参数接口
interface OnClickSlowRankOpCallbackParams {
    startValue: number;
    endValue: number;
    rankId: number;
    name: string;
}

type RectItemValues = [number, number, number, number, number, string];

const DEFAULT_CHART_HEIGHT = 460;
const DEFAULT_INNER_CHART_HEIGHT = 300;
const DEFAULT_CHART_ZOOM_HEIGHT = 400;
const MIN_CHART_ITEM_HEIGHT = 30;
const MAX_CHART_HEIGHT = 800;
const NS_TO_MS_FACTOR = 0.000001;
// Communication 缩略图初始化最大可见rank数量
const INITINAL_MAX_VISIBLE_RANK_NUMBER = 516;
// Communication 缩略图初始化最大可见算子数
const MAX_VISIBLE_OPERATOR_NUMBER = 10000;
// 支持Y轴定位的最小数量
const START_POSITION_AXIS_Y = 20;

// 計算位置
function initDataZoom(totalNum: number, dataLength: number, communicationChartZoomData?: ChartZoomData): void {
    if (dataLength <= 0 || totalNum <= 0 || option.dataZoom.length <= 1) {
        return;
    }
    // 计算 Communication 缩略图纵轴显示范围，限定最多显示516列，如果rank数量超过516则计算比例，计算 范围 = (516 ÷ rank数量) * 100
    // 显示区间为[100 - 范围, 100]
    if (dataLength > INITINAL_MAX_VISIBLE_RANK_NUMBER) {
        const yPercentage = Math.ceil(INITINAL_MAX_VISIBLE_RANK_NUMBER / dataLength * 100);
        option.dataZoom[1].start = 100 - yPercentage;
        option.dataZoom[1].end = 100;
    } else {
        option.dataZoom[1].start = 0;
        option.dataZoom[1].end = 100;
    }
    // 计算 Communication 缩略图横轴显示范围，限定最多显示10000条（10000是估值，实际显示范围会有所波动），计算 范围 = (10000 ÷ 数据总量) * 100
    // 显示区间为[0, 范围]
    if (totalNum > MAX_VISIBLE_OPERATOR_NUMBER) {
        const xPercentage = Math.ceil(MAX_VISIBLE_OPERATOR_NUMBER / totalNum * 100);
        option.dataZoom[0].start = communicationChartZoomData?.start ?? 0;
        option.dataZoom[0].end = communicationChartZoomData?.end ?? xPercentage;
    } else {
        option.dataZoom[0].start = communicationChartZoomData?.start ?? 0;
        option.dataZoom[0].end = communicationChartZoomData?.end ?? 100;
    }
}
enum compareSource {
    COMPARISON = 0,
    BASELINE = 1,
}
const sourceIndex = 4;
function wrapData(dataSource: AnalysisChartData, isCompare: boolean, communicationChartZoomData?: ChartZoomData): any {
    const data: any = [];
    const yAxisData: string[] = [];
    const dataLength = Math.max(dataSource?.data?.length, 0);
    const theme = themeInstance.getThemeType();
    let totalNumber = 0;
    for (let i = dataLength - 1; i >= 0; --i) {
        totalNumber += dataSource.data[i].lists.compare.length;
        const rankId = dataSource.data[i].rankId;
        yAxisData.push(rankId);
        dataSource.data[i].lists?.compare.forEach((item, _) => {
            data.push(getRenderData({ item, rankId, theme, source: compareSource.COMPARISON }));
        });
        if (isCompare) {
            dataSource.data[i].lists?.baseline.forEach((item, _) => {
                data.push(getRenderData({ item, rankId, theme, source: compareSource.BASELINE }));
            });
        }
    }
    option.yAxis.data = yAxisData;
    option.xAxis.min = nsToMs(dataSource.minTime);
    option.xAxis.max = nsToMs(dataSource.maxTime);
    const dataHeight = calculateDataHeight(dataSource);
    option.grid.height = dataHeight;
    option.dataZoom[0].top = dataHeight - DEFAULT_INNER_CHART_HEIGHT + DEFAULT_CHART_ZOOM_HEIGHT;
    initDataZoom(totalNumber, dataLength, communicationChartZoomData);
    option.series = getSeries({ data, isCompare });
    option.tooltip = getTooltip({ isCompare });
    return option;
}

const getRenderData = ({ item, rankId, source, theme }: {item: OperatorTimeItem;rankId: string;source: compareSource;theme: Theme}): any => {
    const startTime = nsToMs(item.startTime);
    const duration = nsToMs(item.duration);
    const endTime = startTime + duration;
    return {
        name: `${rankId}-${item.operatorName}`,
        value: [rankId, startTime, endTime, duration, source, item.operatorName],
        itemStyle: {
            normal: {
                color: theme.colorPalette[colorPalette[hashToNumber(item.operatorName, colorPalette.length)]],
            },
        },
    };
};

const baseSeire = {
    type: 'custom',
    itemStyle: {
        opacity: 1,
    },
    encode: {
        x: [1, 2],
        y: 0,
    },
    data: [],
};
function getSeries({ isCompare, data }: {isCompare: boolean;data: any[]}): any[] {
    return [{ ...baseSeire, data, renderItem: getRenderItem(isCompare) }];
}

function getRenderItem(isCompare: boolean): any {
    return (params: any, api: any): any => {
        const categoryIndex = api.value(0);
        const start = api.coord([api.value(1), categoryIndex]);
        const end = api.coord([api.value(2), categoryIndex]);
        const height = api.size([0, 1])[1] * 0.6 * (isCompare ? 0.5 : 1);
        let y;
        if (isCompare) {
            const isComparison = api.value(4) === compareSource.COMPARISON;
            // 对比在上，基线在下
            y = isComparison ? start[1] - height : start[1] + (height / 3);
        } else {
            y = start[1] - (height / 2);
        }
        const rectShape = echarts.graphic.clipRectByRect(
            {
                x: start[0],
                y,
                width: end[0] - start[0],
                height,
            },
            {
                x: params.coordSys.x,
                y: params.coordSys.y,
                width: params.coordSys.width,
                height: params.coordSys.height,
            },
        );
        return (
            {
                type: 'rect',
                transition: ['shape'],
                shape: rectShape,
                name: 'op',
                style: api.style(),
                emphasis: {
                    style: {
                        stroke: '#999999',
                        lineWidth: 1,
                        opacity: 0.6,
                    },
                },
            }
        );
    };
}

function getTooltip({ isCompare }: { isCompare: boolean }): any {
    return {
        formatter: (params: FormatterParams): string => {
            let tooltipMarkup = `${params.marker} `;
            let getName = (val: string): string => val;
            if (isCompare) {
                const isBaseline = params.value[sourceIndex] === compareSource.BASELINE;
                getName = isBaseline ? getBaselineName : getCompareName;
            }
            tooltipMarkup += getTipLineStr('Rank ID', `${params.value[0]}`);
            tooltipMarkup += getTipLineStr(getName('Operator Name'), `${params.value[5]}`);
            tooltipMarkup += getTipLineStr(getName('Start Time'), `${numberToStr(params.value[1])}ms`);
            tooltipMarkup += getTipLineStr(getName('Elapse Time'), `${numberToStr(params.value[3])}ms`);
            return tooltipMarkup;
        },
    };
}
function numberToStr(value: number): string {
    return `${value.toFixed(6).replace(/\.?0+$/, '')}`;
}

function nsToMs(value: number): number {
    return value * NS_TO_MS_FACTOR;
}
function msToNs(value: number): number {
    return Math.round(value / NS_TO_MS_FACTOR);
}

function getTipLineStr(name: string, value: string): string {
    let html = `${i18n.t(`tableHead.${name}`, { ns: 'communication' })}: `;
    html += `<strong style="color: black">${safeStr((`${value}`))}</strong><br/>`;
    return html;
}

const option: any = {
    textStyle: getDefaultChartOptions().textStyle,
    dataZoom: [
        {
            type: 'slider',
            filterMode: 'weakFilter',
            showDataShadow: false,
            top: DEFAULT_CHART_ZOOM_HEIGHT,
            labelFormatter: '',
            start: 0,
            end: 100,
            xAxisIndex: 0,
            bottom: 10,
            height: 20,
            borderColor: '#d2dbee80',
        },
        {
            type: 'slider',
            filterMode: 'weakFilter',
            showDataShadow: false,
            labelFormatter: '',
            start: 0,
            end: 100,
            yAxisIndex: 0,
            right: 10,
            width: 20,
            borderColor: '#d2dbee80',
        },
        {
            type: 'inside',
            filterMode: 'weakFilter',
            zoomOnMouseWheel: 'ctrl',
            moveOnMouseWheel: 'shift',
        },
    ],
    grid: {
        left: 100,
        right: 120,
        height: DEFAULT_INNER_CHART_HEIGHT,
    },
    xAxis: {
        scale: true,
        name: 'Time(ms)',
        axisLabel: {
            formatter: function (val: number) {
                return numberToStr(Math.max(0, val));
            },
        },
    },
    yAxis: {
        data: [],
        name: 'Rank ID',
    },
    series: [],
};

// 定义操作时间项的接口
export interface OperatorTimeItem {
    operatorName: string;
    startTime: number;
    duration: number;
}

// 定义操作时间信息的接口
export interface OperatorTimeInfo {
    rankId: string;
    dbPath: string;
    lists: CompareData<OperatorTimeItem[]>;
}

// 定义分析图表数据的接口
export interface AnalysisChartData {
    minTime: number;
    maxTime: number;
    data: OperatorTimeInfo[];
}

// 定义操作详情的接口
interface OpDetail {
    name: string;
    rankId: number;
    dbPath: string;
    timestamp: number;
    duration: number;
}
let selectedOpDetail: OpDetail | null;

/**
 * 初始化图表并设置相关事件监听
 * @param dataSource 分析图表数据源
 * @param session 会话信息
 * @param setDropDownVisible 设置下拉菜单可见性的函数
 * @returns 返回初始化的ECharts实例或null
 */
function InitCharts(dataSource: AnalysisChartData, session: Session, setDropDownVisible: (_: boolean) => void): echarts.ECharts | null {
    const chartDom = document.getElementById('hccl');
    if (chartDom === null) {
        return null;
    }
    // 释放已存在的自适应ECharts实例
    disposeAdaptiveEchart(chartDom);
    // 获取新的自适应ECharts实例
    const myChart = getAdaptiveEchart(chartDom);
    const rankDbPathMap: Map<string, string> = new Map();
    dataSource?.data?.forEach((item) => rankDbPathMap.set(item.rankId, item.dbPath));
    myChart.on('contextmenu', { element: 'op' }, (e: echarts.ECElementEvent): void => {
        setDropDownVisible(true);
        const [rankId, timestamp, , duration,, operatorName] = e.value as RectItemValues;
        selectedOpDetail = {
            name: operatorName,
            rankId,
            dbPath: rankDbPathMap.get(rankId.toString()) ?? '',
            timestamp: msToNs(timestamp),
            duration: msToNs(duration),
        };
    });
    if (dataSource !== undefined) {
        myChart.setOption(wrapData(dataSource, session.isCompare, session.communicationChartZoomData));
    }

    return myChart;
}

/**
 * 获取图表高度
 * @param dataSource - 分析图表数据
 * @returns 图表高度
 */
function calculateDataHeight(dataSource: AnalysisChartData): number {
    let calculateHeight: number;
    if (dataSource?.data?.length !== undefined) {
        calculateHeight = Math.max(dataSource.data.length * MIN_CHART_ITEM_HEIGHT, DEFAULT_INNER_CHART_HEIGHT);
    } else {
        calculateHeight = DEFAULT_INNER_CHART_HEIGHT;
    }
    return Math.min(MAX_CHART_HEIGHT, calculateHeight);
}
function getChartHeight(dataSource: AnalysisChartData): number {
    return calculateDataHeight(dataSource) + DEFAULT_CHART_HEIGHT - DEFAULT_INNER_CHART_HEIGHT;
}

/**
 * 重定向到时间线
 * @param setDropDownVisible - 设置下拉菜单可见性的函数
 * @returns 无返回值
 */
async function redirectToTimeline(setDropDownVisible: (_: boolean) => void): Promise<void> {
    if (selectedOpDetail === null) {
        return;
    }
    const { name, rankId, dbPath, duration } = selectedOpDetail;
    const params = {
        name,
        rankId: rankId.toString(),
        dbPath,
    };
    try {
        const res = await queryTimelineUnitKernelDetail(params);
        setDropDownVisible(false);
        const resObj = res ?? {};
        connector.send({
            event: 'switchModule',
            body: {
                switchTo: 'timeline',
                toModuleEvent: 'locateUnit',
                params: {
                    ...resObj,
                    ...params,
                    processId: resObj.pid,
                    startTime: resObj.startTime,
                    rankId: resObj.rankId,
                    duration,
                },
            },
        });
    } catch (e) {
        const errMsg = (e as ErrorInfo)?.message;
        setDropDownVisible(false);
        if (errMsg !== undefined) {
            message.error(errMsg);
        }
    }
}

/**
 * 更新时间线加载状态
 * @param isLoading - 是否正在加载
 */
const findInTimelineLoad = (isLoading: boolean): void => {
    const element = document?.getElementById('findInTimeline');
    if (!element) {
        return;
    }

    if (isLoading) {
        element.classList.add('find-in-time-line-load');
    } else {
        element.classList.remove('find-in-time-line-load');
    }
};

/**
 * 获取图表缩放数据
 * @param chartInstance - 图表实例
 * @returns 缩放数据
 */
const getZoomData = (chartInstance: ECharts | null): ChartZoomData => {
    const currentOption = chartInstance?.getOption();
    const { start = 0, end = 100 } = (currentOption?.dataZoom as InsideDataZoomComponentOption[])?.[0] || [];
    return {
        start,
        end,
    };
};

/**
 * 生成菜单项
 * @param session - 会话对象
 * @param setDropDownVisible - 设置下拉菜单可见性的函数
 * @param chartInstance - 图表实例
 * @returns 菜单项数组
 */
const useMenuItems = (session: Session, setDropDownVisible: (_: boolean) => void, chartInstance: ECharts | null): MenuProps['items'] => {
    const { t } = useTranslation('communication');
    const findInTimeline = {
        label: t('Find in Timeline'),
        key: 'findInTimeline',
        id: 'findInTimeline',
        disabled: false,
        onClick: () => {
            findInTimelineLoad(true);
            setTimeout(() => {
                redirectToTimeline(setDropDownVisible);
            });
        },
    };
    const alignOperator = {
        label: t('Align according to selected operator'),
        key: 'alignAccordingToSelectedOperator',
        disabled: false,
        onClick: (): void => {
            setDropDownVisible(false);
            if (selectedOpDetail === null) {
                return;
            }
            session.communicationChartZoomData = getZoomData(chartInstance);
            session.targetOperator = selectedOpDetail as ClickOperatorItem;
        },
    };
    const restoredefault = {
        label: t('Restore default state'),
        key: 'restoreDefaultState',
        disabled: false,
        onClick: (): void => {
            session.communicationChartZoomData = getZoomData(chartInstance);
            session.targetOperator = undefined;
        },
    };
    if (session.unitcount === 0) {
        findInTimeline.disabled = true;
    }
    if (session.targetOperator === undefined) {
        restoredefault.disabled = true;
    }
    return [
        findInTimeline,
        alignOperator,
        restoredefault,
    ];
};

/**
 * Y轴的接口定义。
 *
 * @interface YAxis
 * @property {string[]} data - Y轴的数据，表示为字符串数组。
 * @property {string} [name] - Y轴的名称，可选属性。
 * @property {boolean} [show] - 是否显示Y轴，可选属性。
 */
export interface YAxis {
    data: string[];
    name?: string;
    show?: boolean;
}

/**
 * CommunicationTimeAnalysisChart组件，用于展示通信时间分析图表。
 * @param dataSource - 分析图表的数据源。
 * @param session - 当前会话对象。
 * @param loading - 是否正在加载数据。
 * @returns 返回一个React组件，用于展示通信时间分析图表。
 */
const CommunicationTimeAnalysisChart = observer(({ dataSource, session, loading }: { dataSource: AnalysisChartData; session: Session; loading: boolean}) => {
    // 设置图表高度的state
    const [chartHeight, setChartHeight] = useState(DEFAULT_CHART_HEIGHT);
    // 控制下拉菜单可见性的state
    const [dropDownVisible, setDropDownVisible] = useState(false);
    // 图表容器的引用
    const chartRef = useRef<HTMLDivElement>(null);
    // 页面内容滚动容器
    const scrollContainer = document.querySelector('.mi-page-content');
    // 图表实例的引用
    const chartInst = useRef<echarts.ECharts | null>(null);
    // 获取菜单项
    const menuItems = useMenuItems(session, setDropDownVisible, chartInst.current);

    /**
     * 同步滚动事件处理函数，用于处理鼠标滚轮缩放时的页面滚动问题。
     * @param e - 鼠标滚轮事件对象。
     * @returns 无返回值。
     */
    const syncScroll = (e: WheelEvent): void => {
        // 如果目标元素不是CANVAS，则直接返回
        if ((e.target as HTMLElement).tagName !== 'CANVAS') {
            return;
        }

        // 如果没有按下Ctrl或Shift键，则滚动页面
        if (!e.ctrlKey && !e.shiftKey) {
            scrollContainer?.scrollBy(0, e.deltaY);
        }
    };

    /**
     * 使用useEffect初始化图表
     * @param
     * @returns void
     */
    useEffect(() => {
        setTimeout(() => {
            // 设置图表高度
            setChartHeight(getChartHeight(dataSource));
            // 初始化图表实例
            chartInst.current = InitCharts(dataSource, session, setDropDownVisible);
            // 添加滚轮事件监听
            chartRef.current?.addEventListener('wheel', syncScroll, true);
            // 重置通信图表缩放数据
            session.communicationChartZoomData = undefined;
        });

        // 清理函数，移除滚轮事件监听
        return (): void => {
            chartRef.current?.removeEventListener('wheel', syncScroll, true);
        };
    }, [dataSource]);

    /**
     * 监听并处理慢算子点击事件的函数。
     * @param res - 包含慢算子事件参数的对象，包括起始值、结束值、算子名称和算子ID。
     * @returns 无返回值。
     */
    useEventBus('onClickSlowRankOp', (res): void => {
        // 解构慢算子事件参数
        const { startValue, endValue, name, rankId } = res as OnClickSlowRankOpCallbackParams;
        const dataSourceAxisY = chartInst.current?.getOption().yAxis as YAxis[];
        const dataSourceLength = dataSourceAxisY[0]?.data.length || 0;
        let startAxisY = 0;
        let endAxisY = 100;
        if (dataSourceLength >= START_POSITION_AXIS_Y) {
            const tempSourceIndex = Number.isNaN(rankId) ? 0 : Number(rankId);
            const rankIdNumber = dataSourceAxisY[0].data.findIndex(item => Number(item) === tempSourceIndex);
            // 计算 Y 轴范围，确保范围在 0 到 dataSourceLength 之间
            startAxisY = Math.floor(Math.max(0, rankIdNumber - 10) / dataSourceLength * 100);
            endAxisY = Math.floor(Math.min(dataSourceLength, rankIdNumber + 10) / dataSourceLength * 100);
        }
        // 根据接收到的起始值和结束值调整图表横轴范围
        chartInst.current?.dispatchAction({
            type: 'dataZoom',
            startValue,
            endValue,
        });
        // 执行纵轴数据缩放
        chartInst.current?.dispatchAction({
            type: 'dataZoom',
            dataZoomIndex: 1, // 指定纵轴
            start: startAxisY,
            end: endAxisY,
        });

        // 先取消所有高亮
        chartInst.current?.dispatchAction({
            type: 'downplay',
            seriesIndex: 0,
        });

        // 再高亮具体算子
        chartInst.current?.dispatchAction({
            type: 'highlight',
            seriesIndex: 0,
            name: `${rankId}-${name}`,
        });

        // 延迟取消高亮
        setTimeout(() => {
            chartInst.current?.dispatchAction({
                type: 'downplay',
                seriesIndex: 0,
                name: `${rankId}-${name}`,
            });
        }, 5000);

        // 滚动到视图中心
        chartRef.current?.scrollIntoView({
            block: 'center',
            behavior: 'smooth',
        });
    });

    return session.durationFileCompleted
        ? <Dropdown
            menu={{
                items: menuItems,
                onBlur: (e: React.FocusEvent<HTMLUListElement, Element>): void => {
                    const hasItem = menuItems?.findIndex(item =>
                        (e.relatedTarget as HTMLElement)?.dataset?.menuId?.includes(item?.key as string)) !== -1;
                    if (!hasItem) {
                        setDropDownVisible(false);
                    }
                },
            }}
            trigger={['contextMenu']}
            open={dropDownVisible}
            autoFocus
        >
            <Spin spinning={loading} delay={400}>
                <div ref={chartRef} id={'hccl'} style={{ width: 'calc(100vw - 80px)', height: chartHeight }}></div>
            </Spin>
        </Dropdown>
        : <div style={{ height: '400px' }}><Loading style={{ margin: '200px auto 0' }}/></div>;
});

export default CommunicationTimeAnalysisChart;
