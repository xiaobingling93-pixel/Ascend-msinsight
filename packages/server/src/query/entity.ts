/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

export type CommunicationTimeInfoEntity = {
    iterationId: string;
    stageId: string;
    rankId: string;
    opName: string;
    opSuffix: string;
    elapseTime: number;
    synchronizationTimeRatio: number;
    synchronizationTime: number;
    transitTime: number;
    waitTimeRatio: number;
    waitTime: number;
    idleTime: number;
};

export type CommunicationBandWidthEntity = {
    iterationId: string;
    rankId: string;
    stageId: string;
    opName: string;
    opSuffix: string;
    transportType: string;
    bandwidthSize: number;
    bandwidthUtilization: number;
    largePackageRatio: number;
    sizeDistribution: string;
    transitSize: number;
    transitTime: number;
};

export type KernelDetailEntity = {
    stepId: string;
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
    rankId: string;
    stepId: string;
    stageId: string;
    computingTime: number;
    pureCommunicationTime: number;
    overlapCommunicationTime: number;
    communicationTime: number;
    freeTime: number;
    stageTime: number;
    bubbleTime: number;
    pureCommunicationExcludeReceiveTime: number;
};

export type CommunicationMatrixInfoEntity = {
    groupId: string;
    step: string;
    opName: string;
    groupName: string;
    srcRank: number;
    dstRank: number;
    transportType: string;
    transitSize: number;
    transitTime: number;
    bandwidth: number;
};
