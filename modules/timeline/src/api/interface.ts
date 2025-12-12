/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

// common
export interface PaginationParams {
    pageSize: number;
    current: number;
}

export interface PaginationModel extends PaginationParams {
    count: number;
}

export interface RankInfo {
    clusterId: string;
    host: string;
    rankName: string;
    rankId: string;
    deviceId: string;
}

export interface CardRankInfo {
    rankInfo: RankInfo;
    dbPath: string;
    index?: number;
}

// 算子详情信息
export interface OpDetail {
    id?: string;
    cardId: string;
    dbPath?: string;
    tid: string;
    pid: string;
    depth: number;
    duration: number;
    name: string;
    timestamp: number;
    metaType?: string;
}

export interface UnitDetail {
    cardId: string;
    propsCardId: string;
    pid: string;
}

// parseCards
export interface ParseCardsParam {
    cards: string[];
    dbPaths: string[];
}

// getOverallMetrics
export interface GetOverallMetricsParams extends PaginationParams {
    rankId: string;
    dbPath: string;
    startTime: number;
    endTime: number;
}

export interface GetOverallMetricsResultItem {
    id: string;
    level: number;
    name: string;
    totalTime: number;
    ratio: number;
    nums: number;
    avg: number;
    max: number;
    min: number;
    categoryList: string[];
    children?: GetOverallMetricsResultItem[] | null;
}

export interface GetOverallMetricsResult extends PaginationModel {
    data: GetOverallMetricsResultItem[];
}

// getOverallMetricsMoreList
export interface GetOverallMetricsMoreListParams extends PaginationParams {
    rankId: string;
    dbPath: string;
    categoryList: string[];
    orderBy?: keyof GetOverallMetricsMoreListResultItem;
    order?: 'ascend' | 'descend' | null;
    name?: string;
    startTime: number;
    endTime: number;
}

export interface GetOverallMetricsMoreListResultItem {
    id: string;
    opId?: number; // 算子id，用于算子跳转获取后获取算子详情信息
    timestamp: number;
    duration: number;
    name: string;
}

export interface GetOverallMetricsMoreListResult extends PaginationModel {
    sameOperatorsDetails: GetOverallMetricsMoreListResultItem[];
    rankId: number;
    count: number;
    pageSize: number;
    currentPage: number;
}

export interface SetCardAliasParams {
    rankId: string;
    dbPath: string;
    cardAlias: string;
}

export interface GetAICoreParams {
    rankId: string;
    dbPath: string;
}

export interface GetAICoreParamsResult {
    hasProblem: boolean;
    percent: number;
}

export interface QueryOperatorDispatchParams extends PaginationParams {
    rankId: string;
    dbPath: string;
    orderBy: string;
    order: string;
    startTime: number;
    endTime: number;
};

export interface BaseSummaryRowItemType {
    id: string;
    rankId: string;
    dbPath: string;
    startTime: number;
    duration: number;
    pid: string;
    tid: string;
    depth: number;
    name?: string;
    originOptimizer?: string;
}

interface OperatorDispatchResultItem extends BaseSummaryRowItemType {
    note: string;
}

export interface OperatorDispatchResult extends OperatorDispatchResultItem {
    count: number;
};

export interface EventViewParams {
    currentPage: number;
    pageSize: number;
    rankId: string;
    dbPath: string;
    processName: string;
    orderBy: string;
    metaType: string;
    pid: string;
    tid: string;
    threadIdList?: string[];
    threadName: string;
    order: string;
    filterCondition: string[];
    startTime: number;
    endTime: number;
}

export interface QueryAllSameOperatorsDurationParams {
    rankId: string;
    dbPath: string;
    tid: string[];
    pid: string;
    startTime: number;
    endTime: number;
    name: string;
    wallDuration: number;
    metaTypeList: string[];
    count: number;
    field: string;
    order: string;
    current: number;
    pageSize: number;
    total: number;
    orderBy: string;
}

export interface OpData {
    startTime: string;
    timestamp: number;
    duration: number;
    id: string;
    depth: number;
    tid: string;
    pid: string;
    metaType?: string;
}

export interface QueryAllSameOperatorsDurationResult {
    currentPage: number;
    pageSize: number;
    sameOperatorsDetails: OpData[];
}

export interface QueryCommunicationKernelDetailParams {
    rankId?: string;
    dbPath?: string;
    name: string;
}

export interface CreateCurveParams {
    fileId: string;
    pid: string;
    tid: string;
    x: string;
    y?: string[];
    type?: string;
}

export interface QueryCommunicationKernelDetailResult {
    step: string;
    group: string;
}

export interface CreateCurveResult {
    curveName: string;
}

export interface GetUnitFlowsParams {
    dbPath?: string;
    rankId: string;
    tid: string;
    pid: string;
    startTime: number;
    endTime: number;
    id: string;
    metaType?: string;
    isSimulation: boolean;
}

interface FlowItemInfoPoint {
    depth: number;
    duration: number;
    id: string;
    metaType: string;
    name: string;
    pid: string;
    rankId: string;
    tid: string;
    timestamp: number;
}

interface FlowItemInfo {
    cat: string;
    id: string;
    title: string;
    from: FlowItemInfoPoint;
    to: FlowItemInfoPoint;
}

export interface FlowItem {
    cat: string;
    flows: FlowItemInfo[];
}

export interface GetUnitFlowsResult {
    unitAllFlows: FlowItem[];
}
