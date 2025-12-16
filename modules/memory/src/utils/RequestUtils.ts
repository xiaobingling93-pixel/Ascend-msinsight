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

import type {
    MemoryCurve,
    OperatorMemory,
    OperatorMemoryCondition,
    MemoryType,
    ResourceType,
    StaticMemoryCondition,
    StaticOperatorList,
    StaticOperatorCurve,
    GetTableDataParams,
    GetTableDataResponse,
    MemorySizeQueryCondition,
} from '../entity/memory';
import { GetSlicePositionParams, SlicePositionResponse } from '../entity/memory';
import { GroupBy } from '../entity/memorySession';

/**
 * 查询算子内存数据类型
 * @param params 查询条件
 * @returns { 'dynamic' | 'static' | 'mix' } 查询结果
 */
export const memoryTypeGet = async (params: { rankId: string; dbPath: string }): Promise<MemoryType> => {
    return window.request({ command: 'Memory/view/type', params });
};

export const resourceTypeGet = async (params: { rankId: string; dbPath: string }): Promise<ResourceType> => {
    return window.request({ command: 'Memory/view/resourceType', params });
};

/**
 * 查询算子内存静态表格数据
 * @param params 查询条件
 * @returns {StaticOperatorList[]} 查询结果
 */
export const staticOpMemoryListGet = (params: StaticMemoryCondition): Promise<StaticOperatorList> => {
    return window.request({ command: 'Memory/view/staticOpMemoryList', params: { ...params } });
};

/**
 * 查询算子内存静态曲线数据
 * @param params 查询条件
 * @returns {StaticOperatorGraph[]} 查询结果
 */
export const staticOpMemoryGraphGet = async (params: {
    rankId: string; dbPath: string; graphId: string; modelName?: string; isCompare: boolean;
}): Promise<StaticOperatorCurve> => {
    return window.request({ command: 'Memory/view/staticOpMemoryGraph', params: { ...params } });
};

/**
 * 查询算子内存表格数据
 * @param params 查询条件
 * @returns {OperatorDetail[]} 查询结果
 */
export const operatorsMemoryGet = (params: OperatorMemoryCondition): Promise<OperatorMemory> => {
    return window.request({ command: 'Memory/view/operator', params: { ...params } });
};

interface MemoryCurveGetParams {
    rankId: string;
    dbPath: string;
    type: GroupBy;
    isCompare: boolean;
    start: string;
    end: string;
}

/**
 * 查询算子内存曲线数据
 * @param params 查询条件
 * @returns {OperatorDetail[]} 查询结果
 */
export const memoryCurveGet = async (params: MemoryCurveGetParams): Promise<MemoryCurve> => {
    return window.request({ command: 'Memory/view/memoryUsage', params: { ...params } });
};

/**
 * 查询按组件分组时算子内存表格数据
 * @param params 查询条件
 * @return {GetTableDataResponse} 查询结果
 */
export const fetchTableDataByComponent = async (params: GetTableDataParams):
Promise<GetTableDataResponse> => {
    return await window.request({ command: 'Memory/view/component', params: { ...params } });
};

/**
 * 查询Timeline位置信息
 * @param params 查询条件
 * @return {SlicePositionResponse} 查询结果
 */
export const fetchOperatorPosition = async (params: GetSlicePositionParams):
Promise<SlicePositionResponse> => {
    return await window.request({ command: 'Memory/find/slice', params: { ...params } });
};

/**
 * 查询动态图算子内存最大最小值
 */
export const fetchDynamicOperatorMaxMin = async (params: MemorySizeQueryCondition):
Promise<{minSize: number; maxSize: number}> => {
    return await window.request({ command: 'Memory/view/operatorSize', params: { ...params } });
};

/**
 * 查询静态图算子内存最大最小值
 */
export const fetchStaticOperatorMaxMin = async (params: MemorySizeQueryCondition):
Promise<{minSize: number; maxSize: number}> => {
    return await window.request({ command: 'Memory/view/staticOpMemorySize', params: { ...params } });
};
