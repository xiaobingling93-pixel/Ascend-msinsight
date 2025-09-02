/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

export interface TraceDataType {
    framework: string;
    algorithm: string;
}

export interface SliceItem {
    stageType: string;
    nodeType: 'Task' | 'FP' | 'BP';
    fileId: string; // db路径，跳转时需要这个信息
    name: string;
    startTime: number;
    duration: number;
}

export interface TraceDataItem {
    lists: SliceItem[];
    hostName: string;
    rankId: string;
}

export interface GetTraceDataResults {
    backendType: string;
    framework: string;
    taskData: TraceDataItem[];
    microBatchData: TraceDataItem[];
    stageTypeList: string[];
    minTime: number;
    maxTime: number;
}
