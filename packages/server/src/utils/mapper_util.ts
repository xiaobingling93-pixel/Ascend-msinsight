/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { KernelDetailEntity, StepStatisticEntity } from '../query/entity';

export const mapperToBandWidthEntity = (tempRankId: number, tempOpName: string, tempStepId: string, key: string, transportInfo: any): any => {
    return {
        iterationId: tempStepId,
        rankId: tempRankId,
        opName: tempOpName,
        transportType: key,
        bandwidthSize: transportInfo['Bandwidth(GB/s)'],
        // bandwidthUtilization: transportInfo['Bandwidth(Utilization)'],
        largePackageRatio: transportInfo['Large Packet Ratio'],
        sizeDistribution: JSON.stringify(transportInfo['Size Distribution']),
        transitSize: transportInfo['Transit Size(MB)'],
        transitTime: transportInfo['Transit Time(ms)'],
    };
};

export const mapperToTimeInfoEntity = (tempRankId: number, tempOpName: string, tempStepId: string, tempData: any): any => {
    return {
        iterationId: tempStepId,
        rankId: tempRankId,
        opName: tempOpName,
        elapseTime: tempData['Elapse Time(ms)'],
        idleTime: tempData['Idle Time(ms)'],
        startTime: tempData['Start Timestamp(us)'],
        synchronizationTimeRatio: tempData['Synchronization Time Ratio'],
        synchronizationTime: tempData['Synchronization Time(ms)'],
        transitTime: tempData['Transit Time(ms)'],
        waitTimeRatio: tempData['Elapse Time(ms)'],
        waitTime: tempData['Elapse Time(ms)'],
    };
};

export function mapperToKernelDetail(arr: any[]): KernelDetailEntity {
    return {
        name: arr[0],
        type: arr[1],
        acceleratorCore: arr[2],
        startTime: arr[3],
        duration: arr[4],
        waitTime: arr[5],
        blockDim: arr[6],
        inputShapes: arr[7],
        inputDataTypes: arr[8],
        inputFormats: arr[9],
        outputShapes: arr[10],
        outputDataTypes: arr[11],
        outputFormats: arr[12],
    };
};

export function mapperToStepStatisticsInfo(arr: any[]): StepStatisticEntity {
    return {
        stepId: arr[0],
        rankId: arr[1] === 'rank' ? arr[2] : '',
        stageId: arr[1] === 'stage' ? arr[2] : '',
        computingTime: arr[3],
        pureCommunicationTime: arr[4],
        overlapCommunicationTime: arr[5],
        communicationTime: arr[6],
        freeTime: arr[7],
        stageTime: arr[8],
        bubbleTime: arr[9],
        pureCommunicationExcludeReceiveTime: arr[10],
    };
};
