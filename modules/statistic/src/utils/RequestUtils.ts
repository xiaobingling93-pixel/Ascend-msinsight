/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import type {
    Curve,
    TableInfo,
    TableCondition, Groups,
} from '../entity/curve';
export const tableDataGet = async (params: TableCondition): Promise<TableInfo> => {
    return await window.request({ command: 'IE/table/view', params });
};

export const curveGet = async (params: { rankId: string; type: string; isZh?: boolean}): Promise<Curve> => {
    return await window.request({ command: 'IE/usage/view', params });
};

export const groupGet = async (params: { rankId: string }): Promise<Groups> => {
    return await window.request({ command: 'IE/group', params });
};
