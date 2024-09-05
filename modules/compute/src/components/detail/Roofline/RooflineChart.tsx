/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useRef, useState } from 'react';
import { observer } from 'mobx-react';
import { chartColors, COLOR, safeStr, useWatchDomResize } from 'ascend-utils';
import i18n from 'ascend-i18n';
import { cloneDeep } from 'lodash';
import type { Point, IRooflineChart } from './Index';
import * as echarts from 'echarts';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { useTheme, type Theme } from '@emotion/react';

const baseOption: any = {
    title: {
        left: 'center',
        textStyle: {
            color: COLOR.Grey20,
        },
    },
    color: chartColors,
    tooltip: {
        confine: true,
        axisPointer: {
            type: 'cross',
        },
    },
    legend: {
        tooltip: {
            show: true,
            formatter: function () {
                const div = document.createElement('div');
                div.className = 'legend-tooltip';
                div.append(i18n.t('chart:switchTooltip'));
                return div;
            },
        },
        textStyle: {
            color: COLOR.Grey20,
        },
        top: 30,
    },
    grid: {
        bottom: '30',
        containLabel: true,
    },
    xAxis: {
        type: 'log',
        name: 'Ops/Byte',
        nameTextStyle: {
            color: COLOR.Grey20,
        },
        axisLabel: {
            color: COLOR.Grey40,
        },
    },
    yAxis: {
        type: 'log',
        name: 'TOps/s',
        axisLabel: {
            color: COLOR.Grey40,
        },
        nameTextStyle: {
            color: COLOR.Grey20,
        },
        axisPointer: {
            type: 'shadow',
        },
    },
};

function wrapData(originData: IRooflineChart, theme: Theme): any {
    const series: any[] = [];
    const legendData: any[] = [];
    const transInfo = getRoofInfo(originData);
    if (transInfo !== null) {
        const { maxAxisX, minAxis } = transInfo;
        originData.rooflines.forEach((roofline, index) => {
            const { bw, computility, bwName, point, ratio, computilityName } = roofline;
            const allPositive = bw > 0 && computility > 0 && point[0] > 0 && point[1] > 0;
            if (allPositive) {
            // 斜线公式 y = kx
                const crossPoint = bw > 1 ? [minAxis, bw * minAxis] : [minAxis / bw, minAxis];
                const turningPoint: Point = [computility / bw, computility];
                const rightPoint: Point = [maxAxisX, computility];
                const rooflinePoints = [crossPoint, turningPoint, rightPoint];
                series.push({
                    name: bwName,
                    type: 'scatter',
                    symbolSize: 16,
                    itemStyle: {
                        color: chartColors[(index % chartColors.length)],
                    },
                    emphasis: {
                        scale: 1.2,
                    },
                    data: [[...point, bw, bwName, ratio, point, computilityName]],
                });
                series.push({
                    name: bwName,
                    type: 'line',
                    lineStyle: { width: 2, color: chartColors[(index % chartColors.length)] },
                    data: rooflinePoints,
                    emphasis: {
                        lineStyle: { width: 4 },
                    },
                });
                legendData.push({ name: bwName });
            }
        });
    }
    const option = cloneDeep(baseOption);
    option.title.text = originData.title;
    option.series = series;
    option.legend.data = legendData;
    option.tooltip.formatter = getTooltipFormatter();
    option.title.textStyle.color = theme.textColorSecondary;
    option.legend.textStyle.color = theme.textColorSecondary;
    option.xAxis.nameTextStyle.color = theme.textColorTertiary;
    option.yAxis.nameTextStyle.color = theme.textColorTertiary;
    return option;
}

// roofline 公式 y = kx
function getRoofInfo(data: IRooflineChart): {
    maxAxisX: number;
    minAxis: number;
} | null {
    if (data.rooflines.length === 0) {
        return null;
    }
    // 数据点和转折点
    const allPoints = data.rooflines.reduce<Point[]>((pre, roofline) => {
        const { point, bw, computility } = roofline;
        const allPositive = point[0] > 0 && point[1] > 0 && bw > 0 && computility > 0;
        if (allPositive) {
            const turningPoint: Point = [computility / bw, computility];
            pre.push(point, turningPoint);
        }
        return pre;
    }, []);
    if (allPoints.length === 0) {
        return null;
    }

    let minX = Number.MAX_VALUE;
    let minY = Number.MAX_VALUE;
    let maxX = 0;
    let maxY = 0;
    allPoints.forEach(point => {
        minX = Math.min(point[0], minX);
        minY = Math.min(point[1], minY);
        maxX = Math.max(point[0], maxX);
        maxY = Math.max(point[1], maxY);
    });

    const maxAxisX = getDigit(maxX, 2);
    const minAxis = getDigit(Math.min(minX, minY, 1), -1);
    return {
        maxAxisX,
        minAxis,
    };
}

// 返回比num小数位差diff位的数字，如 (0.002,0) -> 0.001
export function getDigit(num: number, diff = 0): number {
    if (num <= 0) {
        return 0.1;
    }
    const decimalCount = Math.floor(Math.log10(num));
    return Math.pow(10, decimalCount + diff);
}

function getTooltipFormatter(): (p: any) => string {
    return (params: any) => {
        if (params.data !== undefined && params.seriesType === 'scatter') {
            const [, , bw, bwName, ratio, point] = params.data;
            return `<div>
        <div>${params.marker}${safeStr(bwName)}</div>
        <div>${i18n.t('Bandwidth', { ns: 'details' })}: ${safeStr(bw)}TB/s</div>
        <div>${i18n.t('Intensity', { ns: 'details' })}: ${safeStr(point[0])}Ops/Byte</div>
        <div>${i18n.t('Performance', { ns: 'details' })}: ${safeStr(point[1])}TOps/s</div>
        <div>${i18n.t('Performance Ratio', { ns: 'details' })}: ${Number((100 * ratio).toFixed(2))}%</div>
        </div>`;
        } else {
            return '';
        }
    };
}

function InitCharts(data: IRooflineChart, chartDom: HTMLElement | null, theme: Theme): echarts.ECharts | undefined {
    if (chartDom === null || chartDom === undefined || chartDom?.offsetParent === null) {
        return undefined;
    }
    const newChart = echarts.getInstanceByDom(chartDom)
        ? echarts.getInstanceByDom(chartDom)
        : echarts.init(chartDom);
    if (newChart !== undefined) {
        newChart.setOption(wrapData(data, theme));
    }
    return newChart;
}

const RooflineChart = observer(({ dataSource }: { dataSource: IRooflineChart}): JSX.Element => {
    const theme = useTheme();
    const ref = useRef(null);
    const [chart, setChart] = useState<echarts.ECharts | undefined>(undefined);

    // 监听宽度变化
    useWatchDomResize(ref.current, (domRect) => {
        if (domRect.width <= 0) {
            return;
        }
        if (chart !== undefined && chart !== null) {
            chart.resize();
        } else {
            const newChart = InitCharts(dataSource, ref.current, theme);
            setChart(newChart);
        }
    });

    useEffect(() => {
        const newChart = InitCharts(dataSource, ref.current, theme);
        setChart(newChart);
    }, [dataSource, theme]);
    return (
        <div ref={ref} style={{ height: '500px', width: '100%' }}>
        </div>
    );
});

export const RooflineChartGroup = ({ dataSource }: {
    dataSource: IRooflineChart[];
}): JSX.Element => {
    const theme = useTheme();
    return <div>
        {
            dataSource.slice(0, 1).map(item => (<RooflineChart key={item.title} dataSource={item}/>))
        }
        {
            dataSource.length > 1
                ? (
                    <CollapsiblePanel title={i18n.t('More')} collapsible defaultOpen={false}
                        headerStyle={{ color: theme.primaryColor, fontSize: '12px', cursor: 'pointer', paddingLeft: '12px' }}
                        contentStyle={{ padding: 0, width: 'calc( 100% + 50px)', marginLeft: '-50px' }}
                        style={{ margin: '0 0 20px 50px' }}
                    >
                        {dataSource.slice(1).map(item => (<RooflineChart key={item.title} dataSource={item}/>))}
                    </CollapsiblePanel>
                )
                : <></>
        }
    </div>;
};

export default RooflineChart;
