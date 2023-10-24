/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

/**
 * 返回的数据是否包含memory信息
 * @interface RankInfo
 */
export interface RankInfo {
    /**
     *
     * @type {string}
     * @memberof RankInfo
     */
    rankId: string;
    /**
     *
     * @type {boolean}
     * @memberof RankInfo
     */
    hasMemory: boolean;
};

/**
 * 内存曲线
 * @interface OperatorMemory
 */
export interface MemoryCurve {
    /**
     *
     * @type {string}
     * @memberof MemoryCurve
     */
    peakMemoryUsage: string;
    /**
     *
     * @type {Array<Array<number | string>>}
     * @memberof MemoryCurve
     */
    lines: Array<Array<number | string>>;
    /**
     *
     * @type {string}
     * @memberof MemoryCurve
     */
    token?: string;
    /**
     *
     * @type {boolean}
     * @memberof MemoryCurve
     */
    hasApp: boolean;
}

/**
 *
 * @interface OperatorMemory
 */
export interface OperatorMemory {
    /**
     *
     * @type {OperatorDetail[]}
     * @memberof OperatorMemory
     */
    operatorDetail: OperatorDetail[];
    /**
     *
     * @type {string}
     * @memberof OperatorMemory
     */
    token: string;
    /**
     *
     * @type {number}
     * @memberof OperatorMemory
     */
    totalNum: number;
}

/**
 *
 * @interface OperatorDetail
 */
export interface OperatorDetail {
    /**
     *
     * @type {string}
     * @memberof OperatorDetail
     */
    key?: string;
    /**
     *
     * @type {string}
     * @memberof OperatorDetail
     */
    name: string;
    /**
     *
     * @type {number}
     * @memberof OperatorDetail
     */
    size: number;
    /**
     *
     * @type {number}
     * @memberof OperatorDetail
     */
    allocationTime: number;
    /**
     *
     * @type {number}
     * @memberof OperatorDetail
     */
    releaseTime: number;
    /**
     *
     * @type {number}
     * @memberof OperatorDetail
     */
    duration: number;
}

/**
 *
 * @interface Graph
 */
export interface Graph {
    /**
     *
     * @type {string}
     * @memberof Graph
     */
    title?: string;
    /**
     *
     * @type {string[]}
     * @memberof Graph
     */
    columns: string[];
    /**
     *
     * @type {Array<Array<number | string>>}
     * @memberof Graph
     */
    rows: Array<Array<number | string>>;
}

export interface MemoryTable {
    columns: MemoryTableColumn[];
    rows: OperatorDetail[];
}

export interface MemoryTableColumn {
    name: string;
    type: string;
    key: string;
}

/**
 * 算子内存表格查询条件
 * @interface OperatorMemoryCondition
 */
export interface OperatorMemoryCondition {
    /**
     * rankId
     */
    rankId: string;
    /**
     * 开始时间
     */
    startTime?: number;
    /**
     * 结束时间
     */
    endTime?: number;
    /**
     * token信息
     */
    token: string;
    /**
     * 当前页
     */
    currentPage: number;
    /**
     * 每页显示条目
     */
    pageSize: number;
    /**
     * 排序方式
     */
    order?: string;
    /**
     * 排序列名
     */
    orderBy?: string;
    /**
     * 最小内存
     */
    minSize: number;
    /**
     * 最大内存
     */
    maxSize: number;
    /**
     * 算子名称筛选条件
     */
    orderName?: string;
}
