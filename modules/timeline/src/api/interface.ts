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

// 算子详情信息
export interface OpDetail {
    id: number;
    cardId: string;
    tid: string;
    pid: string;
    depth: number;
    duration: number;
    name: string;
    timestamp: number;
}

// parseCards
export interface ParseCardsParam {
    cards: string[];
}

// getOverallMetrics
export interface GetOverallMetricsParams extends PaginationParams {
    rankId: string;
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
    categoryList: string[];
    orderBy?: keyof GetOverallMetricsMoreListResultItem;
    order?: 'ascend' | 'descend' | null;
    name?: string;
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
    cardAlias: string;
}

export interface GetAICoreParams {
    rankId: string;
}

export interface GetAICoreParamsResult {
    hasProblem: boolean;
    percent: number;
}

export interface QueryOperatorDispatchParams extends PaginationParams {
    rankId: string;
    orderBy: string;
    order: string;
};

interface OperatorDispatchResultItem {
    id: string;
    rankId: string;
    startTime: number;
    duration: number;
    pid: string;
    tid: string;
    depth: number;
    name: string;
    note: string;
}

export interface OperatorDispatchResult extends OperatorDispatchResultItem {
    count: number;
};
