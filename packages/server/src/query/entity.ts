/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

export type CommunicationTimeInfoEntity = {
    iterationId: number;
    rankId: number;
    opName: string;
    elapseTime: number;
    synchronizationTimeRatio: number;
    synchronizationTime: number;
    transitTime: number;
    waitTimeRatio: number;
    waitTime: number;
};

export type CommunicationBandWidthEntity = {
    iterationId: number;
    rankId: number;
    opName: string;
    transportType: string;
    bandwidthSize: number;
    bandwidthUtilization: number;
    largePackageRatio: number;
    sizeDistribution: string;
    transitSize: number;
    transitTime: number;
};

export type KernelDetailEntity = {
    name: string;
    type: string;
    acceleratorCore: string;
    startTime: number;
    duration: number;
    waitTime: number;
    blockDim: number;
    inputShapes: string;
    inputDataTypes: string;
    inputFormats: string;
    outputShapes: string;
    outputDataTypes: string;
    outputFormats: string;
};

export type StepStatisticEntity = {
    rank_id: string;
    step_id: string;
    stage_id: string;
    computing_time: number;
    pure_communication_time: number;
    overlap_communication_time: number;
    communication_time: number;
    free_time: number;
    stage_time: number;
    bubble_time: number;
    pure_communication_exclude_receive_time: number;
};
