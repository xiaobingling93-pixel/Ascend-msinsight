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

export interface ShortParam {
    rankId: string;
    dbPath: string;
    group: string;
    topK: number;
}
export type StaticParam = ShortParam & {
    current: number;
    pageSize: number;
    orderBy: string;
    order: string;
};

export type DetailParam = StaticParam & {
    opType: string;
    shape: string;
    opName?: string;
    accCore?: string;
};
/**
 * 查询算子类型耗时占比
 *
 * @param {rankId} rankId RankID
 * @param {number} group
 * @param {number} topK
 * @return {[]}
 */
export const queryOperatorCategory = async(param: ShortParam): Promise<any> => {
    return window.requestData('operator/category', param);
};

/**
 * 查询算子计算单元耗时
 *
 * @param {rankId} rankId RankID
 * @param {number} group
 * @param {number} topK
 * @return {[]}
 */
export const queryOperatorComputeUnit = async(param: ShortParam): Promise<any> => {
    return window.requestData('operator/compute_unit', param);
};
/**
 * 分页查询算子
 *
 * @param {rankId} rankId RankID
 * @param {number} group
 * @param {number} topK
 * @param {number} current 分页
 * @param {number} pageSize
 * @param {number} orderBy 排序
 * @param {number} order
 * @param {Array} filters
 * @param {boolean} isCompare
 * @return {total:number;data:[]}
 */
export const queryOperators = async(param: StaticParam): Promise<any> => {
    return window.requestData('operator/details', param);
};

/**
 * 分页查询算子统计详情
 *
 * @param {rankId} rankId RankID
 * @param {number} group
 * @param {number} topK
 * @param {number} current 分页
 * @param {number} pageSize
 * @param {number} orderBy 排序
 * @param {number} order
 * @param {Array} filters
 * @param {boolean} isCompare
 * @return {total:number;data:[]}
 */
export const queryOperatorStatic = async(param: StaticParam): Promise<any> => {
    return window.requestData('operator/statistic', param);
};

/**
 * 分页查询统计项（算子类型、算子类型加inputshape）下所有算子
 *
 * @param {rankId} rankId RankID
 * @param {number} group
 * @param {number} topK
 * @param {number} current 分页
 * @param {number} pageSize
 * @param {number} orderBy 排序
 * @param {number} order
 * @param {string} opType 算子类型
 * @param {string} shape
 * @param {Array} filters
 * @return {total:number;data:[]}
 */
export const queryOperatorsInStatic = async(param: DetailParam): Promise<any> => {
    return window.requestData('operator/more_info', param);
};

/**
 * 导出算子详情
 *
 * @param {rankId} rankId RankID
 * @param {number} group
 * @param {number} topK
 * @param {boolean} isCompare
 * @return {exceedingFileLimit:boolean;filePath:string}
 */
export const exportOperatorDetail = async(param: ShortParam & { isCompare: boolean }): Promise<any> => {
    return window.requestData('operator/exportDetails', param);
};
