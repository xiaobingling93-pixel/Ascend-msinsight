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
