/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

export interface Curve {
    legends: string[];
    lines: Array<Array<number | string>>;
}

interface Group {
    label: string;
    value: string;
}

export interface Groups {
    groups: Group[];
}
export interface TableInfo {
    operatorDetail: any[];
    columnAttr: TableColumn[];
    totalNum: number;
}

export interface DataDetail {
    id?: string;
    name: string;
    startTime?: string;
    duration?: string;
}

/**
 *
 * @interface Graph
 */
export interface Graph {
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

export interface CurveTable {
    columns: TableColumn[];
    rows: DataDetail[];
}

export interface TableColumn {
    name: string;
    type: string;
    key: string;
}
export interface TableCondition {
    /**
     * rankId
     */
    rankId: string;
    /**
     * 开始时间
     */
    startTime?: string;
    /**
     * 结束时间
     */
    endTime?: string;
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
     * group名
     */
    type: string;
}

export type RenderExpandRecord = DataDetail;
