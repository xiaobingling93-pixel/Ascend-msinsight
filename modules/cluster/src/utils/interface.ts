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
    algorithm: 'megatron-lm(tp-cp-ep-dp-pp)' | 'megatron-lm(tp-cp-pp-dp-ep)' | 'mindspeed(tp-cp-ep-dp-pp)' | 'mindie-llm(tp-dp-ep-pp-moetp)' | 'vllm(tp-pp-dp-ep)';
    level: string;
    dpSize: number;
    ppSize: number;
    tpSize: number;
    epSize: number;
    cpSize: number;
    moeTpSize: number | null;
}

export type SetParallelStrategyParams = Omit<GetParallelStrategyRes, 'level'>;

export type ParallelismType = 'ep' | 'dp' | 'cp' | 'pp' | 'tp' | 'moeTp';
export type ConnectionType = 'exp' | 'dp' | 'cp' | 'pp' | 'tp' | 'moeTp';
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
    type: ConnectionType;
    list: number[];
    group: string[];
}

export interface ArrangementItem {
    index: number;
    name: string;
    position: { x: number; y: number };
    indicator: Record<string, number>;
    formattedRanks: string;
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

export interface ImportExpertDataParams {
    filePath: string;
    version: 'unbalanced' | 'balanced';
}

export interface QueryExpertHotspotParams {
    layerNum: number;
    expertNum: number;
    modelStage: 'prefill' | 'decode';
    version: 'unbalanced' | 'balanced';
    denseLayerList: number[];
}

export interface QueryExpertHotspotItem {
    modelStage: 'prefill' | 'decode';
    rankId: number;
    visits: number;
    layer: number;
    expertId: number;
    expertIndex: number;
    version: 1 | 2; // 1: 均衡前  2: 均衡后
}

export interface QueryExpertHotspotResult {
    hotspotInfos: QueryExpertHotspotItem[];
}

export interface QueryModelInfoResult {
    layerNum: number;
    expertNum: number;
    denseLayerList: number[];
}

export interface WrapBandwidthDataParams {
    domId: string;
    iterationId: number;
    rankId: number;
    operatorName: string;
    stage: string;
    isDark: boolean;
    pgName: string;
}

export interface PacketAndBandwidthChartsParams extends WrapBandwidthDataParams {
    locale: string;
}
