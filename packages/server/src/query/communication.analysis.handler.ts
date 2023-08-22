/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import {
    AllOperatorsResponse,
    BandwidthDataRequest,
    BandwidthDataResponse,
    DistributionDataRequest,
    DistributionResponse,
    DurationListRequest,
    DurationResponse,
    Durations,
    IterationsOrRanksObject,
    IterationsOrRanksResponse,
    OperatorDetailsRequest,
    OperatorNamesRequest,
    OperatorsObject,
    OperatorsResponse,
    RanksRequest,
} from './communicationAnalysisData';
import { CLUSTER_DATABASE } from '../database/tableManager';
import { getLoggerByName } from '../logger/loggger_configure';
const logger = getLoggerByName('communication', 'info');

export const iterationsHandler = async (): Promise<IterationsOrRanksResponse> => {
    const response: IterationsOrRanksResponse = { iterationsOrRanks: [] };
    response.iterationsOrRanks = await CLUSTER_DATABASE.queryIterationIds() as IterationsOrRanksObject[];
    if (response.iterationsOrRanks.length === 0) {
        logger.error('Failed to obtain the number of iteration ids. At least one id must be contained. ' +
            'Check whether communication data files exist in the directory.');
    }
    return response;
};

export const ranksHandler = async (request: RanksRequest): Promise<IterationsOrRanksResponse> => {
    const response: IterationsOrRanksResponse = { iterationsOrRanks: [] };
    response.iterationsOrRanks = await CLUSTER_DATABASE.queryRankIds(request.iterationId) as IterationsOrRanksObject[];
    return response;
};

export const operatorNamesHandler = async (request: OperatorNamesRequest): Promise<OperatorsResponse> => {
    const response: OperatorsResponse = { operators: [] };
    response.operators = await CLUSTER_DATABASE.selectOperators(request.iterationId,
        request.rankList) as OperatorsObject[];
    return response;
};

export const durationListHandler = async (request: DurationListRequest): Promise<DurationResponse> => {
    const response: DurationResponse = { duration: [] };
    response.duration = await CLUSTER_DATABASE.queryDurationList(request.iterationId,
        request.rankList, request.operatorName) as Durations[];
    return response;
};

export const operatorDetailsHandler = async (request: OperatorDetailsRequest): Promise<AllOperatorsResponse> => {
    const operatorNumber = await CLUSTER_DATABASE.queryOperatorsCount(request.iterationId, request.rankId);
    const totalOpInfoNumber = await CLUSTER_DATABASE.queryTotalOpInfoCount(request.iterationId, request.rankId);
    if (operatorNumber.length !== 1 || totalOpInfoNumber.length !== 1) {
        logger.error('select operator counts error.');
    }
    const orderByName = [ '', 'elapse_time', 'transit_time', 'synchronization_time',
        'wait_time', 'synchronization_time_ratio', 'wait_time_ratio', 'idle_time' ];
    if (!orderByName.includes(request.orderBy)) {
        logger.error('The sort field entered is incorrect.');
    }
    const params: OperatorDetailsRequest = {
        iterationId: request.iterationId,
        rankId: request.rankId,
        pageSize: request.pageSize,
        currentPage: request.currentPage,
        orderBy: request.orderBy,
        order: request.order === 'ascend' ? 'ASC' : 'DESC',
    };
    const response: AllOperatorsResponse = {
        count: operatorNumber[0].nums - totalOpInfoNumber[0].nums,
        pageSize: request.pageSize,
        currentPage: request.currentPage,
        allOperators: [],
    };
    response.allOperators = await CLUSTER_DATABASE.queryAllOperators(params);
    return response;
};

export const bandwidthHandler = async (request: BandwidthDataRequest): Promise<BandwidthDataResponse> => {
    const response: BandwidthDataResponse = { bandwidthData: [] };
    response.bandwidthData = await CLUSTER_DATABASE.queryBandwidthData(request.iterationId, request.rankId,
        request.operatorName);
    if (response.bandwidthData.length !== 4) {
        logger.error('select bandwidth data error. Four types of communication data, ' +
            'such as HCCS, PCIe, SDMA, and RDMA, should be returned.');
    }
    return response;
};

export const distributionHandler = async (request: DistributionDataRequest): Promise<DistributionResponse> => {
    const response: DistributionResponse = { distributionData: '' };
    response.distributionData = await CLUSTER_DATABASE.queryDistributionData(request.iterationId,
        request.rankId, request.operatorName, request.transportType) as string;
    return response;
};
