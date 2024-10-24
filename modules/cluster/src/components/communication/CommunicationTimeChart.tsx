/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
import { observer } from 'mobx-react-lite';
import React, { useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { Spin } from 'ascend-components';
import { chartVisbilityListener, COLOR, commonEchartsOptions } from '../Common';
import type { Session } from '../../entity/session';
import i18n from 'ascend-i18n';
import { cloneDeep } from 'lodash';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { chartColors, getAdaptiveEchart, getDefaultChartOptions } from 'ascend-utils';

function InitCharts(data: dataType): void {
    const chartDom = document.getElementById('main');
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    const myChart = getAdaptiveEchart(chartDom);
    myChart.setOption(wrapData(data));
}
function wrapData(data: dataType): any {
    const options = cloneDeep(baseOption);
    options.legend.data = baseOption.legend.data.map((item: any) => ({
        ...item,
        name: i18n.t(`tableHead.${item.name}`, { ns: 'communication' }),
    }));
    options.series = baseOption.series.map((item: any) => ({
        ...item,
        name: i18n.t(`tableHead.${item.name}`, { ns: 'communication' }),
    }));
    options.xAxis[0].data = data.rankId;
    const order: Array<keyof dataType> = ['elapseTime', 'transitTime', 'synchronizationTime',
        'waitTime', 'synchronizationTimeRatio', 'waitTimeRatio'];
    for (let i = 0; i < options.series.length; i++) {
        options.series[i].data = data[order[i]] ?? [];
    }
    return options;
}

const baseOption: any = {
    textStyle: getDefaultChartOptions().textStyle,
    color: chartColors,
    tooltip: {
        ...commonEchartsOptions.tooltip,
        confine: true,
    },
    toolbox: {
        show: false,
    },
    legend: {
        bottom: 0,
        data: [
            { name: 'Elapse Time', textStyle: { color: COLOR.GREY_50 } },
            { name: 'Transit Time', textStyle: { color: COLOR.GREY_50 } },
            { name: 'Synchronization Time', textStyle: { color: COLOR.GREY_50 } },
            { name: 'Wait Time', textStyle: { color: COLOR.GREY_50 } },
            { name: 'Synchronization Time Ratio', textStyle: { color: COLOR.GREY_50 } },
            { name: 'Wait Time Ratio', textStyle: { color: COLOR.GREY_50 } }],
        tooltip: {
            show: true,
            formatter: function () {
                const div = document.createElement('div');
                div.className = 'legend-tooltip';
                div.append(i18n.t('chart:switchTooltip'));
                return div;
            },
        },
    },
    xAxis: [
        {
            type: 'category',
            data: [],
            axisPointer: {
                type: 'shadow',
            },
            axisLabel: {
                color: COLOR.GREY_40,
            },
        },
    ],
    yAxis: [
        {
            type: 'value',
            name: 'Time(ms)',
            axisLabel: {
                formatter: '{value}',
                color: COLOR.GREY_40,
            },
        },
        {
            type: 'value',
            name: 'Ratio',
            axisLabel: {
                formatter: '{value}',
                color: COLOR.GREY_40,
            },
            splitLine: commonEchartsOptions.splitLineY,
        },
    ],
    series: [
        {
            name: 'Elapse Time',
            type: 'bar',
            tooltip: {
                valueFormatter: (value: any): string => {
                    return `${value}ms`;
                },
            },
            data: [],
        },
        {
            name: 'Transit Time',
            type: 'bar',
            tooltip: {
                valueFormatter: (value: any): string => {
                    return `${value}ms`;
                },
            },
            data: [],
        },
        {
            name: 'Synchronization Time',
            type: 'bar',
            tooltip: {
                valueFormatter: (value: any): string => {
                    return `${value}ms`;
                },
            },
            data: [],
        },
        {
            name: 'Wait Time',
            type: 'bar',
            tooltip: {
                valueFormatter: (value: any): string => {
                    return `${value}ms`;
                },
            },
            data: [],
        },
        {
            name: 'Synchronization Time Ratio',
            type: 'line',
            yAxisIndex: 1,
            tooltip: {
                valueFormatter: (value: any): string => {
                    return value;
                },
            },
            data: [],
        },
        {
            name: 'Wait Time Ratio',
            type: 'line',
            yAxisIndex: 1,
            tooltip: {
                valueFormatter: (value: any): string => {
                    return value;
                },
            },
            data: [],
        },
    ],
    grid: {
        left: 100,
        right: 100,
    },
};

export interface dataType {
    [name: string]: any;
    rankId: string[];
    elapseTime?: number[];
    transitTime?: number[];
    synchronizationTime?: number[];
    waitTime?: number[];
    synchronizationTimeRatio?: number[];
    waitTimeRatio?: number[];
}

const CommunicationTimeChart = observer(({ dataSource, session }: {dataSource: dataType; session: Session}) => {
    const { t } = useTranslation('communication');
    chartVisbilityListener('main', () => {
        InitCharts(dataSource);
    });
    useEffect(() => {
        setTimeout(() => {
            InitCharts(dataSource);
        });
    }, [dataSource, t]);
    return (
        <CollapsiblePanel title={t('sessionTitle.VisualizedCommunicationTime')}>
            <Spin spinning={session.clusterCompleted && !session.durationFileCompleted } tip="">
                <div id={'main'} style={{ height: '400px' }} ></div>
            </Spin>
        </CollapsiblePanel>
    );
});

export default CommunicationTimeChart;
