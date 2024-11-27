/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import {
    chartColors,
    COLOR,
    safeStr,
    formatDecimal,
    getLegendStyle,
    useWatchDomResize,
    getDefaultChartOptions,
} from 'ascend-utils';
import i18n from 'ascend-i18n';
import { cloneDeep } from 'lodash';
import type { Point, IRooflineChart } from './Index';
import * as echarts from 'echarts';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { useTheme, type Theme } from '@emotion/react';
import { LimitHit } from '../../LimitSet';
import { useTranslation } from 'react-i18next';

const baseOption: any = {
    textStyle: getDefaultChartOptions().textStyle,
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
        trigger: 'axis',
        enterable: true,
    },
    legend: {
        type: 'scroll',
        orient: 'horizontal',
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
        left: 'center',
        itemGap: 15,
    },
    grid: {
        bottom: '30',
        containLabel: true,
        top: 100,
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
        nameGap: 25,
        axisPointer: {
            type: 'shadow',
        },
    },
};

type Option = typeof baseOption;

function wrapData(originData: IRooflineChart, theme: Theme): Option {
    const series: any[] = [];
    const legendData: any[] = [];
    const transInfo = getRoofInfo(originData);
    if (transInfo !== null) {
        const { maxAxisX, minAxis } = transInfo;
        // 算力名
        let labelPoint: Point | undefined;
        let computilityNameLabel: string | undefined;
        originData.rooflines.forEach((roofline, index) => {
            const { bw, computility, bwName, point, ratio, computilityName } = roofline;
            const allPositive = bw > 0 && computility > 0 && point?.[0] > 0 && point?.[1] > 0;
            if (!allPositive) {
                return;
            }
            // 斜线公式 y = kx
            const crossPoint: Point = bw > 1 ? [minAxis, bw * minAxis] : [minAxis / bw, minAxis];
            const turningPoint: Point = [computility / bw, computility];
            const rightPoint: Point = [maxAxisX, computility];
            const rooflinePoints: Point[] = [crossPoint, turningPoint, rightPoint];
            series.push(getPointSerie(bwName, index, [...point, bw, bwName, ratio, point, computilityName]));
            series.push(getRooflineSerie(bwName, index, rooflinePoints));
            legendData.push({ name: bwName });
            // 算力名
            if (labelPoint === undefined) {
                labelPoint = rightPoint;
                computilityNameLabel = roofline.computilityName;
            }
        });

        // 算力名
        if (labelPoint !== undefined && computilityNameLabel !== undefined) {
            series.push(getComputilityNameSerie(labelPoint, computilityNameLabel ?? '', theme));
        }
    }

    const option = cloneDeep(baseOption);
    option.title.text = originData.title;
    option.series = series;
    option.legend.data = legendData;
    option.tooltip.formatter = getTooltipFormatter();
    return getOptionStyle(option, theme);
}

function getPointSerie(bwName: string, index: number, data: Array<number | string | Point>): any {
    return {
        name: bwName,
        itemStyle: {
            color: chartColors[(index % chartColors.length)],
        },
        data: [data],
        type: 'scatter',
        symbolSize: 16,
        emphasis: {
            scale: 1.2,
        },
        zlevel: 2,
    };
}

function getRooflineSerie(bwName: string, index: number, rooflinePoints: Point[]): any {
    return {
        name: bwName,
        lineStyle: { width: 2, color: chartColors[(index % chartColors.length)] },
        data: rooflinePoints,
        type: 'line',
        emphasis: {
            lineStyle: { width: 4 },
        },
        zlevel: 1,
    };
}

function getComputilityNameSerie(labelPoint: Point, computilityNameLabel: string, theme: Theme): any {
    return {
        name: 'computilityName',
        type: 'scatter',
        symbolSize: 0,
        data: [labelPoint],
        emphasis: { disabled: true },
        label: {
            position: 'insideBottomRight',
            distance: 5,
            show: true,
            formatter: safeStr(computilityNameLabel ?? ''),
            padding: 10,
            fontSize: 14,
            color: theme.textColor,
            fontWeight: 'bold',
        },
    };
}
function getOptionStyle(option: Option, theme: Theme): Option {
    option.title.textStyle.color = theme.textColorSecondary;
    option.legend = { ...option.legend, ...getLegendStyle(theme) };
    option.xAxis.nameTextStyle.color = theme.textColorTertiary;
    option.yAxis.nameTextStyle.color = theme.textColorTertiary;
    // 网格样式
    option.xAxis.splitLine = {
        lineStyle: {
            color: [theme.borderColorLight],
        },
    };
    option.yAxis.splitLine = {
        lineStyle: {
            color: [theme.borderColorLight],
        },
    };
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
    return (params: any[]) => {
        const tips = params.map(item => getTipText((item))).join('');
        return tips === '' ? '' : `<div style="max-height:400px;overflow-y:auto;">${tips}</div>`;
    };
}

const getTipText = (params: any): any => {
    if (params.data !== undefined && params.seriesType === 'scatter' && params.seriesName !== 'computilityName') {
        const keepDecimalNum = 3;
        const [, , bw, bwName, ratio, point] = params.data;
        return `<div>
        <div>${params.marker}${safeStr(bwName)}</div>
        <div>${i18n.t('Bandwidth', { ns: 'details' })}: ${safeStr(formatDecimal(bw, keepDecimalNum))}TB/s</div>
        <div>${i18n.t('Intensity', { ns: 'details' })}: ${safeStr(formatDecimal(point[0], keepDecimalNum))}Ops/Byte</div>
        <div>${i18n.t('Performance', { ns: 'details' })}: ${safeStr(formatDecimal(point[1], keepDecimalNum))}TOps/s</div>
        <div>${i18n.t('Performance Ratio', { ns: 'details' })}: ${safeStr(formatDecimal((100 * Number(ratio)), keepDecimalNum))}%</div>
        </div>`;
    } else {
        return '';
    }
};

function InitChart(data: IRooflineChart, chartDom: HTMLElement | null, theme: Theme): void {
    if (!chartDom) {
        return;
    }
    const newChart = echarts.getInstanceByDom(chartDom)
        ? echarts.getInstanceByDom(chartDom)
        : echarts.init(chartDom);
    newChart?.setOption(wrapData(data, theme), { replaceMerge: ['series'] });
}

const MAX_ROOFLINE_NUM = 100;
const RooflineChart = observer(({ dataSource: orginDataSource }: { dataSource: IRooflineChart}): JSX.Element => {
    const theme = useTheme();
    const { t } = useTranslation('details');
    const [width, ref] = useWatchDomResize<HTMLDivElement>('width');
    // 超大数据量防护
    const [limit, setLimit] = useState({ maxSize: MAX_ROOFLINE_NUM, overlimit: false, current: 0 });
    const size = orginDataSource?.rooflines?.length ?? 0;
    const dataSource = useMemo(() => ({
        ...orginDataSource,
        rooflines: orginDataSource.rooflines.slice(0, limit.maxSize),
    }), [orginDataSource, limit.maxSize]);

    useEffect(() => {
        setLimit({ ...limit, overlimit: size > limit.maxSize, current: size });
    }, [size]);

    useEffect(() => {
        InitChart(dataSource, ref.current, theme);
    }, [dataSource, theme]);
    useEffect(() => {
        if (ref.current === null || width <= 0) {
            return;
        }
        echarts.getInstanceByDom(ref.current)?.resize();
    }, [width]);
    return (
        <div>
            {limit.overlimit && <LimitHit maxSize={limit.maxSize} name={`${dataSource.title} ${t('Roofline Data')} (${limit.current})`} style={{ margin: '10px 0 10px 50px' }} />}
            <div ref={ref} style={{ height: '500px', width: '100%' }}> </div>
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
