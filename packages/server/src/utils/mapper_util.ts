/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { KernelDetailEntity, StepStatisticEntity } from '../query/entity';
import { toInteger } from 'lodash';

export const mapperToBandWidthEntity = (tempRankId: number, tempOpName: string, tempStepId: string,
    key: string, transportInfo: any, stageId: string): any => {
    const tempOpNameArr = tempOpName.split('@');
    return {
        iterationId: tempStepId,
        stageId,
        rankId: tempRankId,
        opName: tempOpNameArr[0],
        opSuffix: tempOpNameArr[1],
        transportType: key,
        bandwidthSize: transportInfo['Bandwidth(GB/s)'],
        largePackageRatio: transportInfo['Large Packet Ratio'],
        sizeDistribution: JSON.stringify(transportInfo['Size Distribution']),
        transitSize: transportInfo['Transit Size(MB)'],
        transitTime: transportInfo['Transit Time(ms)'],
    };
};

export const mapperToTimeInfoEntity = (tempRankId: number, tempOpName: string, tempStepId: string,
    tempData: any, stageId: string): any => {
    const tempOpNameArr = tempOpName.split('@');
    return {
        iterationId: tempStepId,
        stageId,
        rankId: tempRankId,
        opName: tempOpNameArr[0],
        opSuffix: tempOpNameArr[1],
        elapseTime: tempData['Elapse Time(ms)'],
        idleTime: tempData['Idle Time(ms)'],
        startTime: tempData['Start Timestamp(us)'],
        synchronizationTimeRatio: tempData['Synchronization Time Ratio'],
        synchronizationTime: tempData['Synchronization Time(ms)'],
        transitTime: tempData['Transit Time(ms)'],
        waitTimeRatio: tempData['Wait Time Ratio'],
        waitTime: tempData['Wait Time(ms)'],
    };
};

export const mapperToMatrixInfoEntity = (tempPath: any, temp: any): any => {
    const tempName = tempPath[2].split('@');
    const rankIds = tempPath[3].split('-');
    return {
        groupId: tempPath[0],
        iterationId: tempPath[1],
        opName: tempName[0],
        groupName: tempName[1],
        srcRank: rankIds[0],
        dstRank: rankIds[1],
        transportType: temp[0],
        transitSize: temp[1],
        transitTime: temp[2],
        bandwidth: temp[3],
    };
};

export function mapperToKernelDetail(arr: any[], map: Map<string, number>): KernelDetailEntity {
    const stepIndex = map.get('Step Id');
    const nameIndex = map.get('Name');
    const typeIndex = map.get('Type');
    const acceleratorIndex = map.get('Accelerator Core');
    const startTimeIndex = map.get('Start Time(us)');
    const durationIndex = map.get('Duration(us)');
    const waitTimeIndex = map.get('Wait Time(us)');
    const blockDimIndex = map.get('Block Dim');
    const inputShapesIndex = map.get('Input Shapes');
    const inputDataIndex = map.get('Input Data Types');
    const inputFormatsIndex = map.get('Input Formats');
    const outputIndex = map.get('Output Shapes');
    const outputDataIndex = map.get('Output Data Types');
    const outputFormatsIndex = map.get('Output Formats');
    return {
        stepId: stepIndex === undefined ? '' : arr[stepIndex],
        name: nameIndex === undefined ? '' : arr[nameIndex],
        type: typeIndex === undefined ? '' : arr[typeIndex],
        acceleratorCore: acceleratorIndex === undefined ? '' : arr[acceleratorIndex],
        startTime: startTimeIndex === undefined ? 0 : arr[startTimeIndex],
        duration: durationIndex === undefined ? 0 : arr[durationIndex],
        waitTime: waitTimeIndex === undefined ? 0 : arr[waitTimeIndex],
        blockDim: blockDimIndex === undefined ? 0 : arr[blockDimIndex],
        inputShapes: inputShapesIndex === undefined ? '' : arr[inputShapesIndex],
        inputDataTypes: inputDataIndex === undefined ? '' : arr[inputDataIndex],
        inputFormats: inputFormatsIndex === undefined ? '' : arr[inputFormatsIndex],
        outputShapes: outputIndex === undefined ? '' : arr[outputIndex],
        outputDataTypes: outputDataIndex === undefined ? '' : arr[outputDataIndex],
        outputFormats: outputFormatsIndex === undefined ? '' : arr[outputFormatsIndex],
    };
};

export function mapperToStepStatisticsInfo(arr: any[]): StepStatisticEntity {
    return {
        stepId: toInteger(arr[0]).toString(),
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
