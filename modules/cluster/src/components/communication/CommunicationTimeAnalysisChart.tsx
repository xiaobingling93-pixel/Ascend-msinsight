/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import * as echarts from 'echarts';

import type { Session } from '../../entity/session';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react-lite';
import { addResizeEvent, Loading, safeStr } from '../Common';
import { colorPalette, hashToNumber } from '../../utils/colorUtil';
import { Dropdown } from 'lib/components';
import type { MenuProps } from 'antd';
import connector from '../../connection';
import CollapsiblePanel from 'lib/CollapsiblePanel';
import i18n from 'lib/i18n';

const DEFAULT_CHART_HEIGHT = 460;
const DEFAULT_INNER_CHART_HEIGHT = 300;
const DEFAULT_CHART_ZOOM_HEIGHT = 400;
const MIN_CHART_ITEM_HEIGHT = 30;
const MAX_CHART_HEIGHT = 800;
const NS_TO_MS_FACTOR = 0.000001;
function wrapData(dataSource: AnalysisChartData): any {
    const data: any = [];
    const yAxisData: string[] = [];
    const dataLength = Math.max(dataSource?.data?.length, 0);
    for (let i = dataLength - 1; i >= 0; --i) {
        const rankId = dataSource.data[i].rankId;
        yAxisData.push(rankId);
        dataSource.data[i].lists?.forEach((item, _) => {
            const startTime = nsToMs(item.startTime);
            const duration = nsToMs(item.duration);
            const endTime = startTime + duration;
            data.push(
                {
                    name: item.operatorName,
                    value: [rankId, startTime, endTime, duration],
                    itemStyle: {
                        normal: {
                            color: colorPalette[hashToNumber(item.operatorName, colorPalette.length)],
                        },
                    },
                },
            );
        });
    }
    option.yAxis.data = yAxisData;
    option.xAxis.min = nsToMs(dataSource.minTime);
    option.xAxis.max = nsToMs(dataSource.maxTime);
    const dataHeight = calculateDataHeight(dataSource);
    option.grid.height = dataHeight;
    option.dataZoom[0].top = dataHeight - DEFAULT_INNER_CHART_HEIGHT + DEFAULT_CHART_ZOOM_HEIGHT;
    option.series[0].data = data;
    return option;
}

function renderItem(params: any, api: any): any {
    const categoryIndex = api.value(0);
    const start = api.coord([api.value(1), categoryIndex]);
    const end = api.coord([api.value(2), categoryIndex]);
    const height = api.size([0, 1])[1] * 0.6;
    const rectShape = echarts.graphic.clipRectByRect(
        {
            x: start[0],
            y: start[1] - (height / 2),
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
        }
    );
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
    tooltip: {
        formatter: function (params: {marker: any; name: any; value: any[] }) {
            let tooltipMarkup = `${params.marker} `;
            tooltipMarkup += getTipLineStr('Rank ID', `${params.value[0]}`);
            tooltipMarkup += getTipLineStr('Operator Name', `${params.name}`);
            tooltipMarkup += getTipLineStr('Start Time', `${numberToStr(params.value[1])}ms`);
            tooltipMarkup += getTipLineStr('Elapse Time', `${numberToStr(params.value[3])}ms`);
            return tooltipMarkup;
        },
    },

    dataZoom: [
        {
            type: 'slider',
            filterMode: 'weakFilter',
            showDataShadow: false,
            top: DEFAULT_CHART_ZOOM_HEIGHT,
            labelFormatter: '',
        },
        {
            type: 'slider',
            filterMode: 'weakFilter',
            showDataShadow: false,
            left: '95%',
            yAxisIndex: 0,
            labelFormatter: '',
        },
        {
            type: 'inside',
            filterMode: 'weakFilter',
        },
    ],
    grid: {
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
    series: [
        {
            type: 'custom',
            renderItem,
            itemStyle: {
                opacity: 0.8,
            },
            encode: {
                x: [1, 2],
                y: 0,
            },
            data: [],
        },
    ],
};

export interface OperatorTimeItem {
    operatorName: string;
    startTime: number;
    duration: number;
}
export interface OperatorTimeInfo {
    rankId: string;
    lists: OperatorTimeItem[];
}

export interface AnalysisChartData {
    minTime: number;
    maxTime: number;
    data: OperatorTimeInfo[];
}

interface OpDetail {
    name: string;
    rankId: number;
    timestamp: number;
    duration: number;
}
let selectedOpDetail: OpDetail | null;

function InitCharts(dataSource: AnalysisChartData, session: Session, setDropDownVisible: (_: boolean) => void): void {
    const chartDom = document.getElementById('hccl');
    if (chartDom === null) {
        return;
    }
    echarts.init(chartDom).dispose();
    const myChart = echarts.init(chartDom);
    myChart.on('contextmenu', { element: 'op' }, (e: echarts.ECElementEvent): void => {
        if (session.unitcount === 0) {
            return;
        }
        const [rankId, timestamp, , duration] = e.value as number[];
        selectedOpDetail = {
            name: e.name,
            rankId,
            timestamp: msToNs(timestamp),
            duration: msToNs(duration),
        };
        setDropDownVisible(true);
    });
    if (dataSource !== undefined) {
        myChart.setOption(wrapData(dataSource));
    }
    addResizeEvent(myChart);
}

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

async function redirectToTimeline(): Promise<void> {
    if (selectedOpDetail === null) {
        return;
    }
    const { name, rankId, timestamp, duration } = selectedOpDetail;
    const params = {
        name,
        rankId: rankId.toString(),
        timestamp,
    };
    const res = await window.requestData('unit/one/kernelDetail', params, 'timeline');
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
                startTime: timestamp,
                rankId: resObj.rankId,
                duration,
            },
        },
    });
}

const useMenuItems = (): MenuProps['items'] => {
    const { t } = useTranslation('communication');
    return [
        {
            label: t('Find in Timeline'),
            key: 'findInTimeline',
            onClick: (): void => {
                redirectToTimeline();
            },
        },
    ];
};

const CommunicationTimeAnalysisChart = observer(({ dataSource, session }: { dataSource: AnalysisChartData; session: Session}) => {
    const [chartHeight, setChartHeight] = useState(DEFAULT_CHART_HEIGHT);
    const [dropDownVisible, setDropDownVisible] = useState(false);
    const menuItems = useMenuItems();
    useEffect(() => {
        setTimeout(() => {
            setChartHeight(getChartHeight(dataSource));
            InitCharts(dataSource, session, setDropDownVisible);
        });
    }, [dataSource]);

    return <CollapsiblePanel title={'HCCL'}>
        {session.durationFileCompleted
            ? <Dropdown
                menu={{
                    items: menuItems,
                    onClick: (): void => setDropDownVisible(false),
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
                <div id={'hccl'} style={{ width: 'calc(100vw - 80px)', height: chartHeight }}></div>
            </Dropdown>
            : <div style={{ height: '400px' }}><Loading style={{ margin: '200px auto 0' }}/></div>}
    </CollapsiblePanel>;
});

export default CommunicationTimeAnalysisChart;
