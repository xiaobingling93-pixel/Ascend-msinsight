/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2025. All rights reserved.
*/
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { COLOR, getDecimalCount, getCompareName, getBaselineName } from '../Common';
import type { ConditionDataType } from './Filter';
import type { CompareData, VoidFunction } from '../../utils/interface';
import { queryCommunicationMatrix } from '../../utils/RequestUtils';
import _, { cloneDeep } from 'lodash';
import { type Session } from '../../entity/session';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { safeStr, disposeAdaptiveEchart, getAdaptiveEchart, getDefaultChartOptions } from 'ascend-utils';
import type { TooltipComponentOption, VisualMapComponentOption } from 'echarts/components';
import type { PiecewiseVisualMapOption } from 'echarts/types/dist/shared';
import type { Condition, Range } from './CommunicationMatrix/Filter';
import Filter, { MatrixType } from './CommunicationMatrix/Filter';
import {
    getTransportTypeName,
    getTransportTypeSerie,
    getTransportTypeVisualMap,
} from './CommunicationMatrix/transportType';

interface DataSource {
    data: MatrixItem[];
    rankIds: number[];
}
interface MatrixItem {
    srcRank: number;
    dstRank: number;
    matrixData: CompareData<MatrixData>;
}
interface MatrixData {
    opName: string;
    bandwidth: number;
    transitSize: number;
    transitTime: number;
    transportType: string;
}

interface ChartData {
    data: HeatmapData[];
    rankIds: number[];
    type: MatrixType;
    min: number;
    max: number;
    isCompare: boolean;
}
export type HeatmapData = [string, string, React.Key, CompareData<Record<string, React.Key>>];
export enum HeatmapDataIndex {
    SRC_RANK = 0,
    DST_RANK = 1,
    VALUE = 2,
    DATA = 3,
}

function InitChart(data: ChartData, t: TFunction): void {
    const chartDom = document.getElementById('matrixchart');
    if (chartDom !== null) {
        disposeAdaptiveEchart(chartDom);
        const myChart = getAdaptiveEchart(chartDom);
        myChart.setOption(wrapData(data, t), { replaceMerge: ['series', 'xAxis', 'yAxis'] });
    }
}

function wrapData(dataSource: ChartData, t: TFunction): any {
    const { data, rankIds, type, min, max, isCompare } = dataSource;
    const option: any = cloneDeep(baseOption);
    option.xAxis.data = rankIds;
    option.yAxis.data = rankIds;
    option.series = [getSerie({ data, rankIds, t, type, isCompare })];
    option.visualMap = getVisualMap({ type, min, max, dataLength: data.length, isCompare, t });
    return option;
}

const baseOption: any = {
    xAxis: {
        type: 'category',
        name: 'Src Rank Id',
        splitArea: {
            show: true,
        },
    },
    yAxis: {
        type: 'category',
        name: 'Dst Rank Id',
    },
    tooltip: { show: true },
    textStyle: getDefaultChartOptions().textStyle,
    dataZoom: [
        {
            type: 'inside',
            xAxisIndex: [0],
            start: 0,
            end: 100,
        },
        {
            type: 'inside',
            yAxisIndex: [0],
            start: 0,
            end: 100,
        },
        {
            type: 'inside',
            start: 0,
            end: 100,
        },
    ],
    grid: {
        left: '100',
        right: '100',
        height: '80%',
        top: '10%',
    },
};

export const baseSerie = {
    type: 'heatmap',
    emphasis: {
        itemStyle: {
            shadowBlur: 10,
            shadowColor: COLOR.GREY_50,
        },
    },
};
function getSerie({ data, rankIds, type, isCompare, t }: {
    data: HeatmapData[];
    rankIds: number[];
    type: MatrixType;
    isCompare: boolean;
    t: TFunction;
}): any {
    if (type === MatrixType.TRANSPORT_TYPE) {
        return getTransportTypeSerie({ data, rankIds, type, isCompare, t });
    }
    const { mixData, repeatData } = handleRepeatData(data);
    return {
        ...baseSerie,
        data: mixData,
        label: {
            show: rankIds.length <= 16,
            formatter: function (params: any): string {
                const newData = params?.data;
                const [x, y] = newData;
                const repeatKey = `${x},${y}`;
                return repeatData[repeatKey].length > 1 ? `[${repeatData[repeatKey].join(',')}]` : repeatData[repeatKey][0];
            },
        },
        tooltip: getTooltip({ t, type, repeatData, isCompare }),
    };
}

// 处理重复数据
function handleRepeatData(data: HeatmapData[]): { mixData: HeatmapData[];repeatData: Record<string, string[]> } {
    const repeatData: Record<string, string[]> = {};
    // 此处data中每项均为[srcRank, dstRank, value, ...]形式，存在重复[srcRank, dstRank]时，value拼接为[value1,value2,...]形式在通信矩阵中呈现，并以value1作为颜色指标
    const mixData = data.reduce<HeatmapData[]>((prev, cur) => {
        const repeatKey = `${cur[HeatmapDataIndex.SRC_RANK]},${cur[HeatmapDataIndex.DST_RANK]}`;
        if (repeatData[repeatKey] === undefined) {
            repeatData[repeatKey] = [`${cur[HeatmapDataIndex.VALUE]}`];
        } else {
            repeatData[repeatKey].unshift(`${cur[HeatmapDataIndex.VALUE]}`);
            prev.splice(prev.findIndex(item =>
                item[HeatmapDataIndex.SRC_RANK] === cur[HeatmapDataIndex.SRC_RANK] && item[HeatmapDataIndex.DST_RANK] === cur[HeatmapDataIndex.DST_RANK])
            , 1);
        }
        return [...prev, cur];
    }, []);
    return { mixData, repeatData };
}

interface Label {
    label: string | number;
    content: string | number;
    contentClass?: string;
}
// 提示框
export function getTooltip({ t, type, isCompare, repeatData }: {t: TFunction;type: string;isCompare: boolean;repeatData?: Record<string, string[]>}):
TooltipComponentOption {
    return {
        show: true,
        formatter: function (params: any): string {
            const list = getDisplayList({ t, type, isCompare, repeatData, data: params.data });
            return list.map(labelItem =>
                `<span>${safeStr(labelItem.label)}:</span><span class="tooltip-value ${labelItem.contentClass ?? ''}">${safeStr(labelItem.content)}</span><br/>`,
            ).join('');
        },
    };
}

function getDisplayList({ t, type, isCompare, repeatData, data }:
{data: HeatmapData;t: TFunction;type: string;isCompare: boolean;repeatData?: Record<string, string[]>}): Label[] {
    let [srcRank, dstRank, value, { compare, baseline }] = data;
    const repeatedKey = `${srcRank},${dstRank}`;
    if (type === MatrixType.TRANSPORT_TYPE) {
        value = getTransportTypeName(data, isCompare, true);
    } else if (repeatData !== undefined && repeatData[repeatedKey].length > 1) {
        value = `[${repeatData[repeatedKey].join(', ')}]`;
    }

    const list: Label[] = [{ label: 'srcRank -> dstRank', content: `${srcRank} -> ${dstRank}` }];
    if (isCompare) {
        // 算子名
        if (compare.opName !== '') {
            list.push({ label: t(getCompareName('operatorName')), content: compare.opName });
        }
        if (baseline.opName !== '') {
            list.push({ label: t(getBaselineName('operatorName')), content: baseline.opName });
        }
        if (type !== MatrixType.TRANSPORT_TYPE) {
            list.push({ label: t('Difference'), content: value, contentClass: typeof value === 'number' && value >= 0 ? 'positive-number' : 'negative-number' });
        }
        list.push(
            { label: t(getCompareName(type)), content: compare.value },
            { label: t(getBaselineName(type)), content: baseline.value },
        );
    } else {
        // 算子名
        if (compare.opName !== '') {
            list.push({ label: t('operatorName'), content: compare.opName });
        }
        list.push({ label: t(type), content: value });
    }
    return list;
}

// 图例（颜色条）
const baseVisualMap: PiecewiseVisualMapOption = {
    orient: 'horizontal',
    left: 'center',
    bottom: '0',
    textStyle: { color: COLOR.GREY_40 },
    dimension: 2,
};
function getVisualMap({ dataLength, min, max, type, isCompare = false, t }: {
    dataLength: number;min: number;max: number;isCompare?: boolean;type: MatrixType; t: TFunction;
}): VisualMapComponentOption {
    if (type === MatrixType.TRANSPORT_TYPE) {
        return getTransportTypeVisualMap(isCompare, t);
    }
    if (isCompare) {
        return {
            ...baseVisualMap,
            type: 'piecewise',
            pieces: [
                { lt: 0, color: COLOR.BAND_0 },
                { value: 0, color: COLOR.GREY_20 },
                { gt: 0, color: COLOR.BAND_4 },
            ],
        };
    }
    if (dataLength > 0 || isFinite(max)) {
        return {
            ...baseVisualMap,
            calculable: true,
            itemHeight: 300, // 调整宽度,
            inRange: min === max ? { color: [COLOR.BAND_1] } : { color: [COLOR.BAND_0, COLOR.BAND_1, COLOR.BAND_2, COLOR.BAND_3] },
            min,
            max,
            precision: Math.max(getDecimalCount(min), getDecimalCount(max)),
        };
    }
    return baseVisualMap;
}

// 图表更新
const updateChart = ({ dataSource, switchCondition, range, shouldUpdateRange, setRange, t, isCompare }: {
    dataSource: DataSource;
    switchCondition: Condition;
    range?: Range;
    shouldUpdateRange: boolean;
    setRange: (val: Range) => void;
    t: TFunction;
    isCompare: boolean;
}): void => {
    const { data, rankIds } = dataSource;
    const dataList: HeatmapData[] = data.reduce<HeatmapData[]>((pre, cur) => {
        const { srcRank, dstRank, matrixData: { compare, baseline, diff } } = cur;
        const compareValue = compare[switchCondition.type];
        const baselineValue = baseline[switchCondition.type];
        const diffValue = diff[switchCondition.type];
        const value = isCompare ? diffValue : compareValue;
        let match = rankIds.includes(srcRank) && rankIds.includes(dstRank);
        if (!switchCondition.showInner) {
            match = match && srcRank !== dstRank;
        }
        if (range) {
            match = match && typeof value === 'number' && value >= range.min && value <= range.max;
        }
        if (match) {
            pre.push([String(srcRank), String(dstRank), value,
                {
                    compare: { opName: compare.opName, value: compareValue },
                    baseline: { opName: baseline.opName, value: baselineValue },
                    diff: { value: diffValue },
                },
            ]);
        }
        return pre;
    }, []);
    const values: number[] = dataList.map((item: HeatmapData) => typeof item[HeatmapDataIndex.VALUE] === 'number' ? item[HeatmapDataIndex.VALUE] as number : 0);
    const min = dataList.length > 0 ? Math.min(...values) : 0;
    const max = dataList.length > 0 ? Math.max(...values) : 0;
    if (shouldUpdateRange) {
        setRange({ min, max });
    }
    InitChart({ ...dataSource, data: dataList, type: switchCondition.type, min: range?.min ?? min, max: range?.max ?? max, isCompare }, t);
};

// 数据更新
const updateData = async(condition: ConditionDataType, setDataSource: VoidFunction, isCompare: boolean): Promise<void> => {
    const { iterationId, stage, operatorName, baselineIterationId } = condition;
    const param = { iterationId, stage, operatorName, isCompare, baselineIterationId };
    const res = await queryCommunicationMatrix(param);
    const data = res?.matrixList ?? [];
    const rankIds = _.map(_.split(_.replace(stage, /[(),]/, ''), ','),
        value => Number.parseInt(value)).filter(value => !Number.isNaN(value))
        .sort((a, b) => a - b);
    setDataSource({ data, rankIds });
};

// 通信矩阵
const CommunicationMatrix = observer(({ isShow, conditions, session }: { isShow: boolean;conditions: ConditionDataType;session: Session}) => {
    const { t } = useTranslation('communication');
    const [switchCondition, setSwitchCondition] = useState<Condition>({ type: MatrixType.BANDWIDTH, showInner: false });
    const [range, setRange] = useState<Range>({ min: 0, max: 1 });
    const [dataSource, setDataSource] = useState<DataSource>({ data: [], rankIds: [] });

    const handleFilterChange = (filed: string, val: string | boolean): void => {
        setSwitchCondition({ ...switchCondition, [filed]: val });
    };
    const handleRangeChange = (rangeVal: Range): void => {
        updateChart({ shouldUpdateRange: false, range: rangeVal, setRange, switchCondition, dataSource, t, isCompare: session.isCompare });
    };

    useEffect(() => {
        if (isShow) {
            if (session.clusterCompleted) {
                updateData(conditions, setDataSource, session.isCompare);
            } else {
                setDataSource({ data: [], rankIds: [] });
            }
        }
    }, [isShow, conditions, session.isCompare]);

    useEffect(() => {
        updateChart({ shouldUpdateRange: true, setRange, switchCondition, dataSource, t, isCompare: session.isCompare });
    }, [dataSource, switchCondition, t, session.isCompare]);

    return <CollapsiblePanel style={{ display: isShow ? 'block' : 'none' }} title={t('sessionTitle.MatrixModel')} padding={'16px 24px'}>
        <Filter condition={switchCondition} handleChange={handleFilterChange} range={range} onRangeChange={handleRangeChange}/>
        <div id={'matrixchart'} style={{ width: 'calc(100vw - 80px)', height: '800px' }}></div>
    </CollapsiblePanel>;
});

export default CommunicationMatrix;
