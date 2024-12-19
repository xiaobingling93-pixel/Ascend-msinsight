/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
} from '../entity/memory';
import { GetSlicePositionParams, SlicePositionResponse } from '../entity/memory';

/**
 * 查询算子内存数据类型
 * @param params 查询条件
 * @returns { 'dynamic' | 'static' | 'mix' } 查询结果
 */
export const memoryTypeGet = async (params: { rankId: string }): Promise<MemoryType> => {
    return window.request({ command: 'Memory/view/type', params });
};

export const resourceTypeGet = async (params: { rankId: string }): Promise<ResourceType> => {
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
    rankId: string; graphId: string; modelName?: string; isCompare: boolean;
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

/**
 * 查询算子内存曲线数据
 * @param params 查询条件
 * @returns {OperatorDetail[]} 查询结果
 */
export const memoryCurveGet = async (params: { rankId: string; type: string; isCompare: boolean }): Promise<MemoryCurve> => {
    return window.request({ command: 'Memory/view/memoryUsage', params });
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
