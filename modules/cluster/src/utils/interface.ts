/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

export interface ErrorInfo {
    code: number;
    message?: string;
}

export interface StringMap {
    [prop: string]: string;
}

export interface VoidFunction {
    (...rest: any[]): void;
}

export interface optionDataType {
    key?: string;
    label: React.ReactNode;
    value: number | string ;
}

export interface optionMapDataType {
    [props: string]: optionDataType[];
}

export interface FormatterParams {
    marker: string;
    name: string;
    value: any;
    data: any ;
    seriesName: string;
    seriesType: string;
}

export interface GetParallelStrategyRes {
    algorithm: 'megatron-lm(tp-cp-ep-dp-pp)' | 'megatron-lm(tp-cp-pp-dp-ep)';
    level: string;
    dpSize: number;
    ppSize: number;
    tpSize: number;
    epSize: number;
    cpSize: number;
}

export type SetParallelStrategyParams = Omit<GetParallelStrategyRes, 'level'>;

export type ParallelismType = 'ep' | 'dp' | 'cp' | 'pp' | 'tp';
export interface ParallelismArrangementParams extends SetParallelStrategyParams {
    dimension: 'ep-dp' | 'ep-dp-pp' | 'ep-dp-pp-cp' | 'ep-dp-pp-cp-tp';
}

export interface IndicatorsItem {
    name: string;
    key: string;
    renderHeatMap: boolean;
    renderChart: boolean;
    chart: 'bar' | 'line';
    stack: string;
    yAxisType: 'time' | 'ratio';
    visible: boolean;
    unit: 'μs' | '%';
}

export interface ConnectionsItem {
    type: ParallelismType;
    list: number[];
    group: string[];
}

export interface DomainItem {
    type: ParallelismType;
    list: number[];
}

export interface ArrangementItem {
    index: number;
    name: string;
    position: { x: number; y: number };
    indicator: Record<string, number>;
    attribute: {
        [K in ParallelismType as `${K}Index`]: number;
    };
}

export interface ParallelismArrangementResult {
    size: number;
    arrangements: ArrangementItem[];
    indicators: IndicatorsItem[];
    connections: ConnectionsItem[];
    domains: string[];
}

export interface GetParallelismPerformanceData extends ParallelismArrangementParams {
    indexList?: number[];
    orderBy?: string;
    step?: string;
    isCompare: boolean;
}

export interface PerformanceDataItem {
    [key: string]: any;
    index: number;
}
export interface GetParallelismPerformanceRes {
    performance: PerformanceDataItem[];
    advice: string[];
}

export interface ClickOperatorItem {
    name: string;
    rankId: number;
    timestamp: number;
    duration: number;
}

export interface CompareData<T> {
    compare: T;
    baseline: T;
    diff: T;
}
