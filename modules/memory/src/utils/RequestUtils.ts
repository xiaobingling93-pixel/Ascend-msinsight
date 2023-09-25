/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { MemoryCurve, OperatorMemory, OperatorMemoryCondition } from '../entity/memory';

const dataSource = {
    remote: '127.0.0.1',
    port: 9000,
    dataPath: [],
};

/**
 * 查询算子内存表格数据
 * @param params 查询条件
 * @returns {OperatorDetail[]} 查询结果
 */
export const operatorsMemoryGet = (params: OperatorMemoryCondition): Promise<OperatorMemory> => {
    return window.request(dataSource, { command: 'Memory/view/operator', params: { ...params } });
};

/**
 * 查询算子内存曲线数据
 * @param params 查询条件
 * @returns {OperatorDetail[]} 查询结果
 */
export const memoryCurveGet = async (params: {rankId: string; token: string}): Promise<MemoryCurve> => {
    return window.request(dataSource, { command: 'Memory/view/memoryUsage', params });
};
