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

export interface Curve {
    legends: string[];
    lines: Array<Array<number | string>>;
    description?: string;
    isZh?: boolean;
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
