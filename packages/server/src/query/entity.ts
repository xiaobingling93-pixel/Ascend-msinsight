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
