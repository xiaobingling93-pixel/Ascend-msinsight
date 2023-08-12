/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { KernelDetailEntity } from '../query/entity';

export const mapperToBandWidthEntity = (tempRankId: number, tempOpName: string, key: string, transportInfo: any): any => {
    return {
        iterationId: 1,
        rankId: tempRankId,
        opName: tempOpName,
        transportType: key,
        bandwidthSize: transportInfo['Bandwidth(GB/s)'],
        bandwidthUtilization: transportInfo['Bandwidth(Utilization)'],
        largePackageRatio: transportInfo['Large Packet Ratio'],
        sizeDistribution: JSON.stringify(transportInfo['Size Distribution']),
        transitSize: transportInfo['Transit Size(MB)'],
        transitTime: transportInfo['Transit Time(ms)'],
    };
};

export const mapperToTimeInfoEntity = (tempRankId: number, tempOpName: string, tempData: any): any => {
    return {
        iterationId: 1,
        rankId: tempRankId,
        opName: tempOpName,
        elapseTime: tempData['Elapse Time(ms)'],
        synchronizationTimeRatio: tempData['Elapse Time(ms)'],
        synchronizationTime: tempData['Elapse Time(ms)'],
        transitTime: tempData['Elapse Time(ms)'],
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

export function mapperToStepStatisticsInfo(arr: any[]): KernelDetailEntity {
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
