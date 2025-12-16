/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
} from '@insight/lib/utils';
import i18n from '@insight/lib/i18n';
import { cloneDeep } from 'lodash';
import type { Point, IRooflineChart } from './Index';
import * as echarts from 'echarts';
import { CollapsiblePanel } from '@insight/lib/components';
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
        formatter: (name: string) => {
            name = safeStr(name);
            if (name.startsWith('X(') || name.startsWith('Y(')) {
                return name.slice(2, -1);
            }
            return name;
        },
        textStyle: {
            color: COLOR.Grey20,
        },
        top: 30,
        left: 'center',
        itemGap: 16,
        icon: 'circle',
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

function setCustomLegendOption(theme: Theme): any {
    return {
        icon: 'roundRect',
        top: 62,
        left: 'center',
        right: '12px',
        orient: 'horizontal',
        itemWidth: 16,
        itemHeight: 12,
        itemGap: 16,
        itemStyle: { color: '#6DB9E8' },
        textStyle: { color: theme.textColorTertiary },
        formatter: (name: string): string => {
            name = safeStr(name).slice(2, -1);
            if (name.length > 30) {
                return `${name.slice(0, 28)}...`;
            }
            return name;
        },
        tooltip: {
            show: true,
            formatter: (params: any): string => `<div class="legend-tooltip">${safeStr(params.name).slice(2, -1) ?? ''}</div>`,
        },
    };
}

type Option = typeof baseOption;

interface GetActiveLegendItem {
    selectedOfX: { selected?: {[key: string]: boolean}};
    selectedOfMix: { selected?: {[key: string]: boolean}};
}

/**
 * 初始化时仅展示一个组类的折线和散点
 * @param isInit
 * @param xLegend
 * @param mixLegend
 */
function getActiveLegendWhenInit(isInit: boolean, xLegend: Array<{ [key: string]: any}>, mixLegend: string[]): GetActiveLegendItem {
    const selectedOfXName: { [key: string]: boolean } = {};
    const selectedOfMixName: { [key: string]: boolean } = {};
    if (isInit) {
        let activeName = xLegend[0]?.name ?? '';
        xLegend.forEach(item => {
            selectedOfXName[item.name] = item.name === activeName;
        });
        activeName = activeName.slice(2, -1);
        mixLegend.forEach(name => {
            selectedOfMixName[name] = name.startsWith(activeName);
        });
    }
    const selectedOfX = isInit ? { selected: selectedOfXName } : {};
    const selectedOfMix = isInit ? { selected: selectedOfMixName } : {};
    return { selectedOfX, selectedOfMix };
}

function wrapData(originData: IRooflineChart, theme: Theme, isInit: boolean): Option {
    const series: any[] = [];
    const xLegend: Array<{ [key: string]: any }> = [];
    const yLegend: Array<{ [key: string]: any }> = [];
    const mixLegend: string[] = [];
    const transInfo = getRoofInfo(originData);
    const bwNameList: string[] = [];
    const option = cloneDeep(baseOption);
    if (transInfo !== null) {
        const { maxAxisX, minAxis } = transInfo;
        // 算力名
        let labelPoint: Point | undefined;
        originData.rooflines.forEach((roofline, index) => {
            const { bw, computility, bwName, point, ratio, computilityName } = roofline;
            !bwNameList.includes(bwName) && bwNameList.push(bwName);
            const allPositive = bw > 0 && computility > 0 && point?.[0] > 0 && point?.[1] > 0;
            if (!allPositive) { return; }
            // 斜线公式 y = kx
            const crossPoint: Point = bw > 1 ? [minAxis, bw * minAxis] : [minAxis / bw, minAxis];
            const turningPoint: Point = [computility / bw, computility];
            const rightPoint: Point = [maxAxisX, computility];
            const rooflinePoints: Point[] = [crossPoint, turningPoint, rightPoint];
            series.push(getPointSerie(bwName, [...point, bw, bwName, ratio, point, computilityName], bwNameList));
            series.push(getRooflineSerie(bwName, rooflinePoints, bwNameList, computilityName));
            mixLegend.push(`${bwName}(${computilityName})`);
            !xLegend.find(item => item.name === `X(${bwName})`) && xLegend.push({ name: `X(${bwName})`, itemStyle: { color: getColorByBwName(bwName, bwNameList) } });
            !yLegend.find(item => item.name === `Y(${computilityName})`) && yLegend.push({ name: `Y(${computilityName})` });
            if (labelPoint === undefined) { labelPoint = rightPoint; }
        });
        if (yLegend.length === 1 && labelPoint !== undefined) {
            series.push(getComputilityNameSerie(labelPoint, yLegend[0].name.slice(2, -1) ?? '', theme));
        }
        option.xAxis.min = minAxis;
        option.xAxis.max = maxAxisX;
    }
    option.title.text = originData.title;
    option.series = [...series, ...[...xLegend, ...yLegend].map(item => ({ ...getRooflineSerie(item.name, [], bwNameList, ''), show: false }))];
    const { selectedOfX, selectedOfMix } = getActiveLegendWhenInit(isInit, xLegend, mixLegend);
    // 设置图例
    option.legend = [
        { ...baseOption.legend, ...selectedOfMix, data: mixLegend, show: false },
        { ...baseOption.legend, ...selectedOfX, data: xLegend },
        { ...baseOption.legend, data: yLegend, ...setCustomLegendOption(theme), show: yLegend.length > 1 },
    ];
    option.tooltip.formatter = getTooltipFormatter();
    return getOptionStyle(option, theme);
}

function getColorByBwName(name: string, list: string[]): string {
    const _name = name.startsWith('X(') || name.startsWith('Y(') ? name.slice(2, -1) : name;
    const idx = list.findIndex(item => item === _name) ?? 0;
    return chartColors[(idx % chartColors.length)];
}

function getPointSerie(bwName: string, data: Array<number | string | Point>, bwNameList: string[]): any {
    const computilityName = data[data.length - 1];
    return {
        name: `${bwName}(${computilityName})`,
        itemStyle: { color: getColorByBwName(bwName, bwNameList) },
        data: [data],
        type: 'scatter',
        symbolSize: 16,
        emphasis: { scale: 1.2 },
        zlevel: 2,
    };
}

function getRooflineSerie(bwName: string, rooflinePoints: Point[], bwNameList: string[], computilityName: string): any {
    const compName = computilityName ? `(${computilityName})` : '';
    const color = getColorByBwName(bwName, bwNameList);
    return {
        name: `${bwName}${compName}`,
        lineStyle: { width: 2, color },
        symbol: 'emptyCircle',
        color,
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
    option.legend = option.legend.map((item: any) => ({ ...item, ...getLegendStyle(theme) }));
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
        const [, , bw, bwName, ratio, point, computilityName] = params.data;
        return `<div>
        <div>${params.marker}${safeStr(bwName)}(${computilityName})</div>
        <div>${i18n.t('Bandwidth', { ns: 'details' })}: ${safeStr(formatDecimal(bw, keepDecimalNum))}TB/s</div>
        <div>${i18n.t('Intensity', { ns: 'details' })}: ${safeStr(formatDecimal(point[0], keepDecimalNum))}Ops/Byte</div>
        <div>${i18n.t('Performance', { ns: 'details' })}: ${safeStr(formatDecimal(point[1], keepDecimalNum))}TOps/s</div>
        <div>${i18n.t('Performance Ratio', { ns: 'details' })}: ${safeStr(formatDecimal((100 * Number(ratio)), keepDecimalNum))}%</div>
        </div>`;
    } else {
        return '';
    }
};

function InitChart(data: IRooflineChart, chartDom: HTMLElement | null, theme: Theme, isInit: boolean, setIsInit: React.Dispatch<React.SetStateAction<boolean>>): void {
    if (!chartDom) {
        return;
    }
    const newChart = echarts.getInstanceByDom(chartDom)
        ? echarts.getInstanceByDom(chartDom)
        : echarts.init(chartDom);
    const chartOption = wrapData(data, theme, isInit);
    newChart?.setOption(chartOption, { replaceMerge: ['series'] });
    if (isInit) {
        // 同步默认不高亮图例状态，解决默认设置为不高亮图例图例手动切换到高亮需要点击两次问题
        setTimeout(() => {
            const xLegend = [...chartOption.legend[1].data];
            for (let i = 1; i < xLegend.length; i++) {
                newChart?.dispatchAction({ name: xLegend[i].name, type: 'legendUnSelect' });
            }
        });
        setIsInit(false);
    }
    // 监听图例(legend)的选中状态变化
    newChart?.on('legendselectchanged', (params: any) => {
        const isBwName = params.name.startsWith('X(');
        const status = params.selected[params.name] as boolean;
        const nameOfLines: string[] = [];
        const selectedList: string[] = [];
        Object.keys(params.selected).forEach(name => {
            const isStartX = name.startsWith('X(');
            const isStartY = name.startsWith('Y(');
            if (!isStartX && !isStartY) {
                nameOfLines.push(name);
            } else {
                if (((isBwName && isStartY) || (!isBwName && isStartX)) && params.selected[name]) {
                    selectedList.push(name.slice(2, -1));
                }
            }
        });
        const _name = params.name.slice(2, -1);
        const listOfChanged: string[] = selectedList.map(item => isBwName ? `${_name}(${item})` : `${item}(${_name})`);
        const selected: { [key: string]: boolean } = {};
        // 根据选中的图例的name匹配折线&散点的name来设置线和点显示/隐藏
        nameOfLines.forEach(name => {
            selected[name] = listOfChanged.includes(name) ? status : params.selected[name];
        });
        const chartOption = newChart?.getOption() as any;
        chartOption.legend[0].selected = selected;
        newChart?.setOption(chartOption);
    });
}

const MAX_ROOFLINE_NUM = 100;
const RooflineChart = observer(({ dataSource: orginDataSource }: { dataSource: IRooflineChart}): JSX.Element => {
    const theme = useTheme();
    const { t } = useTranslation('details');
    const [width, ref] = useWatchDomResize<HTMLDivElement>('width');
    // 超大数据量防护
    const [limit, setLimit] = useState({ maxSize: MAX_ROOFLINE_NUM, overlimit: false, current: 0 });
    const [isInit, setIsInit] = useState(true);
    const size = orginDataSource?.rooflines?.length ?? 0;
    const dataSource = useMemo(() => ({
        ...orginDataSource,
        rooflines: orginDataSource.rooflines.slice(0, limit.maxSize),
    }), [orginDataSource, limit.maxSize]);

    useEffect(() => {
        setLimit({ ...limit, overlimit: size > limit.maxSize, current: size });
    }, [size]);

    useEffect(() => {
        InitChart(dataSource, ref.current, theme, isInit, setIsInit);
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
