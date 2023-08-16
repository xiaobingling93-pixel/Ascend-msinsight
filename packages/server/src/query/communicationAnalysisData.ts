/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

export type DurationResponse = {
    duration: Durations[];
};

export type Durations = {
    rankId: string | number;
    elapseTime: number;
    transitTime: number;
    synchronizationTime: number;
    waitTime: number;
    idleTime: number;
    synchronizationTimeRatio: number;
    waitTimeRatio: number;
};

export type AllOperatorsResponse = {
    count: number;
    pageSize: number;
    currentPage: number;
    allOperators: AllOperators[];
};

type AllOperators = {
    operatorName: string;
    elapseTime: number;
    transitTime: number;
    synchronizationTime: number;
    waitTime: number;
    idleTime: number;
    synchronizationTimeRatio: number;
    waitTimeRatio: number;
};

export type BandwidthDataResponse = {
    bandwidthData: BandwidthData[];
};

type BandwidthData = {
    transportType: string;
    transitSize: number;
    transitTime: number;
    bandwidth: number;
    largePacketRatio: number;
};

export type RanksRequest = {
    dbIndex: string;
    iterationId: string;
};

export type OperatorNamesRequest = {
    dbIndex: string;
    iterationId: string;
    rankList: string[];
};

export type DurationListRequest = {
    dbIndex: string;
    iterationId: string;
    rankList: string[];
    operatorName: string;
};

export type OperatorDetailsRequest = {
    dbIndex: string;
    iterationId: string;
    rankId: string;
    pageSize: number;
    currentPage: number;
};

export type BandwidthDataRequest = {
    dbIndex: string;
    iterationId: string;
    rankId: string;
    operatorName: string;
};

export type DistributionDataRequest = {
    dbIndex: string;
    iterationId: string;
    rankId: string;
    operatorName: string;
    transportType: string;
};

export type IterationsOrRanksResponse = {
    iterationsOrRanks: IterationsOrRanksObject[];
};

export type IterationsOrRanksObject = {
    iterationOrRankId: string;
};

export type OperatorsResponse = {
    operators: OperatorsObject[];
};

export type OperatorsObject = {
    operator: string;
};

export type DistributionResponse = {
    distributionData: string;
};
