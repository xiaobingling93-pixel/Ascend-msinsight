/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import {
    AllOperatorsResponse, BandwidthDataResponse, DistributionResponse,
    DurationResponse,
    Durations,
    IterationsOrRanksResponse,
    OperatorsResponse,
} from '../../query/communicationAnalysisData';
import { ClusterDatabase } from '../cluster_database';

const globalDatabase = new ClusterDatabase('./db_tensorboard.db');

it('query iteration ids', async function () {
    const response: IterationsOrRanksResponse = { iterationsOrRanks: [] };
    response.iterationsOrRanks = await globalDatabase.queryIterationIds() as number[];
    expect(response).toEqual({ iterationsOrRanks: [ { iteration_id: 1 }, { iteration_id: 2 } ] });
});

it('query rank ids', async function () {
    const response: IterationsOrRanksResponse = { iterationsOrRanks: [] };
    response.iterationsOrRanks = await globalDatabase.queryRankIds(1) as number[];
    expect(response).toEqual({ iterationsOrRanks: [ { rank_id: 0 }, { rank_id: 1 } ] });
});

it('query rank ids with no data', async function () {
    const response: IterationsOrRanksResponse = { iterationsOrRanks: [] };
    response.iterationsOrRanks = await globalDatabase.queryRankIds(5) as number[];
    expect(response).toEqual({ iterationsOrRanks: [] });
});

it('query all operator names', async function () {
    const response: OperatorsResponse = { operators: [] };
    response.operators = await globalDatabase.selectOperators(1, []) as string[];
    expect(response).toEqual({ operators: [ { op_name: 'send' }, { op_name: 'send1' }, { op_name: 'allReduce' }, { op_name: 'all2' } ] });
});

it('query all operator names with rank list', async function () {
    const response: OperatorsResponse = { operators: [] };
    response.operators = await globalDatabase.selectOperators(1, [0]) as string[];
    expect(response).toEqual({ operators: [ { op_name: 'send' }, { op_name: 'send1' }, { op_name: 'allReduce' } ] });
});

it('query all operator names with no data', async function () {
    const response: OperatorsResponse = { operators: [] };
    response.operators = await globalDatabase.selectOperators(5, [0]) as string[];
    expect(response).toEqual({ operators: [] });
});

it('query duration data', async function () {
    const response: DurationResponse = { duration: [] };
    response.duration = await globalDatabase.queryDurationList(1, [], 'allReduce') as Durations[];
    expect(response).toEqual({
        duration: [
            {
                elapse_time: 1,
                rank_id: 0,
                synchronization_time: 1,
                synchronization_time_ratio: 1,
                transit_time: 1,
                wait_time: 1,
                wait_time_ratio: 1,
            }],
    });
});

it('query duration data with rank list', async function () {
    const response: DurationResponse = { duration: [] };
    response.duration = await globalDatabase.queryDurationList(1, [0], 'allReduce') as Durations[];
    expect(response).toEqual({
        duration: [
            {
                elapse_time: 1,
                rank_id: 0,
                synchronization_time: 1,
                synchronization_time_ratio: 1,
                transit_time: 1,
                wait_time: 1,
                wait_time_ratio: 1,
            }],
    });
});

it('query all Operator details with fenye', async function () {
    const operatorNumber = await globalDatabase.queryOperatorsCount(1, 0);
    console.log(operatorNumber.length);
    const response: AllOperatorsResponse = {
        count: operatorNumber[0].nums,
        pageSize: 1,
        currentPage: 1,
        allOperators: [],
    };
    response.allOperators = await globalDatabase.queryAllOperators(1,
        0, 1, 1);
    expect(response).toEqual({
        count: 3,
        pageSize: 1,
        currentPage: 1,
        allOperators: [
            {
                op_name: 'send',
                elapse_time: 1,
                transit_time: 1,
                synchronization_time: 1,
                wait_time: 1,
                synchronization_time_ratio: 1,
                wait_time_ratio: 1,
            }],
    });
});

it('query bandwidth data', async function () {
    const response: BandwidthDataResponse = { bandwidthData: [] };
    response.bandwidthData = await globalDatabase.queryBandwidthData(1, 0,
        'send');
    console.log(response.bandwidthData.length);
    expect(response).toEqual({
        bandwidthData: [
            {
                transport_type: 'HCCS',
                transit_size: 1,
                transit_time: 1,
                bandwidth_size: 1,
                bandwidth_utilization: 1,
                large_package_ratio: 1,
            },
            {
                transport_type: 'PCIe',
                transit_size: 1,
                transit_time: 1,
                bandwidth_size: 1,
                bandwidth_utilization: 1,
                large_package_ratio: 1,
            },
            {
                transport_type: 'SDMA',
                transit_size: 1,
                transit_time: 1,
                bandwidth_size: 1,
                bandwidth_utilization: 0,
                large_package_ratio: 0,
            },
            {
                transport_type: 'RDMA',
                transit_size: 1,
                transit_time: 1,
                bandwidth_size: 1,
                bandwidth_utilization: 1,
                large_package_ratio: 1,
            } ],
    });
});

it('query distribute data', async function () {
    const response: DistributionResponse = { distributionData: '' };
    response.distributionData = await globalDatabase.queryDistributionData(1,
        0, 'send', 'HCCS');
    const obj = JSON.parse('{ "12": [1, 0.55], "17": [7, 0.35], "22": [10, 0.33] }');
    console.log(obj);
    expect(response).toEqual({
        distributionData: [{
            size_distribution: '{ 12: [1, 0.55], 17: [7, 0.35], 22: [10, 0.33] }',
        }],
    });
    expect(response.distributionData[0]).toEqual({
        size_distribution: '{ 12: [1, 0.55], 17: [7, 0.35], 22: [10, 0.33] }',
    });
});

it('query distribute data with SDMA', async function () {
    const response: DistributionResponse = { distributionData: '' };
    response.distributionData = await globalDatabase.queryDistributionData(1,
        0, 'send', 'SDMA') as string;
    expect(response).toEqual({
        distributionData: [{
            size_distribution: '',
        }],
    });
});
