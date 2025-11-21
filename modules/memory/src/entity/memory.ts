/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { GroupBy } from './memorySession';

export interface RankInfo {
    clusterId: string;
    host: string;
    rankName: string;
    rankId: string;
    deviceId: string;
}

/**
 * 返回的数据是否包含memory信息
 * @interface MemoryRankInfo
 */
export interface MemoryRankInfo {
    /**
     *
     * @type {string}
     * @memberof MemoryRankInfo
     */
    rankId: string;
    /**
     * rank 详情
     * @type {RankInfo}
     * @memberof MemoryRankInfo
     */
    rankInfo: RankInfo;
    /**
     * 数据库路径
     * @type {string}
     * @memberof MemoryRankInfo
     */
    dbPath?: string;
    /**
     *
     * @type {boolean}
     * @memberof MemoryRankInfo
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
    title: string;
    /**
     *
     * @type {string[]}
     * @memberOf MemoryCurve
     */
    legends: string[];
    /**
     *
     * @type {Array<Array<number | string>>}
     * @memberof MemoryCurve
     */
    lines: Array<Array<number | string>>;
}

/**
 * 每条数据详情，包含diff、baseline和compare内容
 */
interface OperatorCompareDetail<T> {
    diff: T;
    baseline: T;
    compare: T;
}

/**
 *
 * @interface OperatorMemory
 */
export interface OperatorMemory {
    /**
     *
     * @type {Array<OperatorCompareDetail<OperatorDetail>>}
     * @memberof OperatorMemory
     */
    operatorDetail: Array<OperatorCompareDetail<OperatorDetail>>;
    /**
     *
     * @type {MemoryTableColumn[]}
     * @memberof OperatorMemory
     */
    columnAttr: MemoryTableColumn[];
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
    id?: string;
    /**
     *
     * @type {string}
     * @memberof OperatorDetail
     */
    name: string;
    /**
     *
     * @type {string}
     * @memberof OperatorDetail
     */
    source?: string;
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
    /**
     *
     * @type {string}
     * @memberof activeReleaseTime
     */
    activeReleaseTime?: string;
    /**
     *
     * @type {number}
     * @memberof activeDuration
     */
    activeDuration?: number;
    /**
     *
     * @type {number}
     * @memberof allocationAllocated
     */
    allocationAllocated?: number;
    /**
     *
     * @type {number}
     * @memberof allocationReserved
     */
    allocationReserved?: number;
    /**
     *
     * @type {number}
     * @memberof allocationActive
     */
    allocationActive?: number;
    /**
     *
     * @type {number}
     * @memberof releaseAllocated
     */
    releaseAllocated?: number;
    /**
     *
     * @type {number}
     * @memberof releaseReserved
     */
    releaseReserved?: number;
    /**
     *
     * @type {number}
     * @memberof releaseActive
     */
    releaseActive?: number;
    /**
     *
     * @type {number}
     * @memberof streamId
     */
    streamId?: string;
    /**
     * 表格展开项内容
     */
    children?: OperatorDetail[];
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
    sortable?: boolean;
    searchable?: boolean;
    rangeFilterable?: boolean;
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
     * dbPath，数据库路径
     */
    dbPath: string;
    /**
     * 开始时间
     */
    startTime?: number;
    /**
     * 结束时间
     */
    endTime?: number;
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
    minSize?: number;
    /**
     * 最大内存
     */
    maxSize?: number;
    /**
     * 是否仅查看在选中时间区间分配或释放内存的数据
     * - 有 startTime, endTime 时生效
     */
    isOnlyShowAllocatedOrReleasedWithinInterval: boolean;
    /**
     * 算子名称筛选条件
     */
    searchName?: string;
    /**
     * 查询的类型：Overall、Stream
     */
    type: GroupBy;
    /**
     * 是否为比对场景
     */
    isCompare: boolean;
    filters?: { [key: string]: string }; // 匹配过滤
    rangeFilters?: { [key: string]: [number, number] }; // 范围过滤
}

/**
 * 静态算子内存表格查询条件
 * @interface StaticMemoryCondition
 */
export interface StaticMemoryCondition {
    /**
     * rankId
     */
    rankId: string;
    /**
     * dbPath，数据库路径
     */
    dbPath: string;
    /**
     * deviceId，通过deviceId条件筛选时需要传入
     */
    deviceId?: string;
    /**
     * modelName，在动态图存在静态子图时需要传入
     */
    modelName?: string;
    /**
     * graphId，默认0，每个graphId单独绘制
     */
    graphId: string;
    /**
     * 开始节点
     */
    startNodeIndex?: number;
    /**
     * 结束节点
     */
    endNodeIndex?: number;
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
    searchName?: string;
    /**
     * 是否为比对场景
     */
    isCompare: boolean;
}

/**
 * 算子内存返回类型
 * @interface MemoryType
 */
export interface MemoryType {
    /**
     *
     * @type {string}
     * @memberof MemoryType
     */
    type: string;
    /**
     *
     * @type {string[]}
     * @memberof MemoryType
     */
    graphId: string[];
}

/**
 * 内存数据来源返回类型
 * @interface MemoryType
 */
export interface ResourceType {
    type: string;
}

/**
 * 静态图表格详细内容的返回类型
 * @interface StaticOperatorListDetail
 */
export interface StaticOperatorListDetail {
    /**
     *
     * @type {string}
     * @memberof StaticOperatorListDetail
     */
    deviceId: string;
    /**
     *
     * @type {string}
     * @memberof StaticOperatorListDetail
     */
    source?: string;
    /**
     *
     * @type {string}
     * @memberof StaticOperatorListDetail
     */
    opName: string;
    /**
     *
     * @type {number}
     * @memberof StaticOperatorListDetail
     */
    nodeIndexStart: number;
    /**
     *
     * @type {number}
     * @memberof StaticOperatorListDetail
     */
    nodeIndexEnd: number;
    /**
     *
     * @type {number}
     * @memberof StaticOperatorListDetail
     */
    size: number;
    /**
     * 静态图表格展开项内容
     */
    children: StaticOperatorListDetail[];
}

/**
 * 静态图表格的返回类型
 * @interface StaticOperatorList
 */
export interface StaticOperatorList {
    /**
     *
     * @type {string[]}
     * @memberof StaticOperatorList
     */
    columnAttr: MemoryTableColumn[];
    /**
     *
     * @type {Array<OperatorCompareDetail<StaticOperatorListDetail>>}
     * @memberof StaticOperatorList
     */
    staticOperatorListDetail: Array<OperatorCompareDetail<StaticOperatorListDetail>>;
    /**
     *
     * @type {number}
     * @memberof StaticOperatorList
     */
    totalNum: number;
}

/**
 * 静态图曲线的返回类型
 * @interface StaticOperatorGraph
 */
export interface StaticOperatorCurve {
    /**
     *
     * @type {string}
     * @memberof StaticOperatorCurve
     */
    title: string;
    /**
     *
     * @type {string[]}
     * @memberOf StaticOperatorCurve
     */
    legends: string[];
    /**
     *
     * @type {Array<Array<number | string>>}
     * @memberof StaticOperatorCurve
     */
    lines: Array<Array<number | string>>;
}

/**
 * 按组件分组时算子内存表格数据
 * @interface ComponentMemory
 */
export interface ComponentMemory {
    key?: number | string;
    name: number;
    peakMemory: number;
    time: number;
    source?: string;
};

/**
 * 按组件分组时算子内存表格数据请求参数
 * @interface GetTableDataParams
 */
export interface GetTableDataParams {
    currentPage: number;
    pageSize: number;
    isCompare: boolean;
    order?: string;
    orderBy?: string;
    rankId?: string;
    dbPath?: string;
}

/**
 * 查找算子在timeline的位置信息
 * @interface GetSlicePositionParams
 */
export interface GetSlicePositionParams {
    id?: string;
    name: string;
    rankId: string;
    dbPath: string;
};

/**
 * 查找算子在timeline的位置信息
 * @interface GetTableDataResponse
 */
export interface SlicePositionResponse {
    id: string;
    rankId: string;
    processId: string;
    threadId: string;
    metaType: string;
    depth: string;
    startTime: number;
    duration: number;
};

/**
 * 按组件分组时算子内存表格数据响应体
 * @interface GetTableDataResponse
 */
export interface GetTableDataResponse {
    totalNum: number;
    columnAttr: MemoryTableColumn[];
    componentDetail: Array<OperatorCompareDetail<ComponentMemory>>;
};

export type RenderExpandRecord = OperatorDetail | ComponentMemory;

/**
 * 按组件分组时算子内存表格数据排序分页
 * @interface OrderPageInfo
 */
export interface OrderPageInfo {
    currentPage: number;
    pageSize: number;
    order?: string;
    orderBy?: string;
};

/**
 * 查询内存视图算子内存最大最小值的条件
 */
export interface MemorySizeQueryCondition {
    rankId: string;
    dbPath: string; // 数据库路径
    type: GroupBy;
    isCompare: boolean;
    graphId?: string;
}
