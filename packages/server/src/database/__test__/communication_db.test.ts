/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import {
    AllOperatorsResponse, BandwidthDataResponse, DistributionResponse,
    DurationResponse,
    Durations,
    IterationsOrRanksObject,
    IterationsOrRanksResponse,
    MatrixResponse,
    OperatorDetailsRequest,
    OperatorsObject,
    OperatorsResponse,
} from '../../query/communicationAnalysisData';
import { ClusterDatabase } from '../cluster_database';
import { CLUSTER_DATABASE } from '../tableManager';
import { StageAndBubbleTimeResponse } from '../../query/data';

const globalDatabase = new ClusterDatabase('cluster.db');

it('query iteration ids', async function () {
    const response: IterationsOrRanksResponse = { iterationsOrRanks: [] };
    response.iterationsOrRanks = await globalDatabase.queryIterationIds() as IterationsOrRanksObject[];
    expect(response).toEqual({ iterationsOrRanks: [ { iteration_id: '2' }, { iteration_id: '3' } ] });
});

it('query rank ids', async function () {
    const response: IterationsOrRanksResponse = { iterationsOrRanks: [] };
    response.iterationsOrRanks = await globalDatabase.queryRankIds('2') as IterationsOrRanksObject[];
    expect(response).toEqual({ iterationsOrRanks: [ { rank_id: '0' }, { rank_id: '1' } ] });
});

it('query rank ids with no data', async function () {
    const response: IterationsOrRanksResponse = { iterationsOrRanks: [] };
    response.iterationsOrRanks = await globalDatabase.queryRankIds('5') as IterationsOrRanksObject[];
    expect(response).toEqual({ iterationsOrRanks: [] });
});

it('query all operator names', async function () {
    const response: OperatorsResponse = { operators: [] };
    response.operators = await globalDatabase.selectOperators('step2', []) as OperatorsObject[];
    expect(response).toEqual({ operators: [ { op_name: 'send' }, { op_name: 'send1' }, { op_name: 'allReduce' }, { op_name: 'all2' } ] });
});

it('query all operator names with rank list', async function () {
    const response: OperatorsResponse = { operators: [] };
    response.operators = await globalDatabase.selectOperators('1', ['0']) as OperatorsObject[];
    expect(response).toEqual({ operators: [ { op_name: 'send' }, { op_name: 'send1' }, { op_name: 'allReduce' } ] });
});

it('query all operator names with no data', async function () {
    const response: OperatorsResponse = { operators: [] };
    response.operators = await globalDatabase.selectOperators('2', ['0']) as OperatorsObject[];
    expect(response).toEqual({ operators: [] });
});

it('query duration data', async function () {
    const response: DurationResponse = { duration: [] };
    response.duration = await globalDatabase.queryDurationList('2', [], 'Total Op Info') as Durations[];
    expect(response).toEqual({
        duration: [
            {
                elapse_time: 1,
                idle_time: -1,
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
    response.duration = await globalDatabase.queryDurationList('1', ['0'], 'allReduce') as Durations[];
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
    const operatorNumber = await globalDatabase.queryOperatorsCount('16', '0');
    const totalOpInfoNumber = await CLUSTER_DATABASE.queryTotalOpInfoCount('16', '0');
    console.log(operatorNumber.length);
    console.log(totalOpInfoNumber.length);
    const response: AllOperatorsResponse = {
        count: operatorNumber[0].nums - totalOpInfoNumber[0].nums,
        pageSize: 1,
        currentPage: 1,
        allOperators: [],
    };
    const param: OperatorDetailsRequest = {
        iterationId: '16',
        rankId: '0',
        pageSize: 10,
        currentPage: 1,
        orderBy: 'elapse_time',
        order: 'DESC',
    };
    response.allOperators = await globalDatabase.queryAllOperators(param);
    expect(response).toEqual({
        count: 4,
        pageSize: 1,
        currentPage: 1,
        allOperators: [
            {
                op_name: 'send',
                elapse_time: 1,
                idle_time: -1,
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
    response.bandwidthData = await globalDatabase.queryBandwidthData('1', '0',
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
    response.distributionData = await globalDatabase.queryDistributionData('1',
        '0', 'send', 'HCCS');
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
    response.distributionData = await globalDatabase.queryDistributionData('1',
        '0', 'send', 'SDMA') as string;
    expect(response).toEqual({
        distributionData: [{
            size_distribution: '',
        }],
    });
});

it('query matrix data', async function () {
    const response: MatrixResponse = { matrixList: [] };
    response.matrixList = await globalDatabase.queryMatrixList('', 'hcom_allReduce__764_0',
        '');
    expect(response).toEqual({
        matrixList: [
            {
                srcRank: '7',
                dstRank: '4',
                transportType: 'hccs',
                transitSize: 0,
                transitTime: 0.0011,
                bandwidth: 0.0354,
            }],
    });
});

it('query group data', async function () {
    const response: [] = await globalDatabase.getGroups();
    const result = response.map((item: any) => item.groupId);
    console.log(response.map((item: any) => item.groupId));
    expect(result).toEqual(
        ['(0, 1, 2, 3, 4, 5, 6, 7)'],
    );
});

it('string split', async function () {
    const str = '13.0,stage,"(0, 1, 2, 3, 4, 5, 6, 7)",294211.5200000002,193649.6699999998,180817.35000000015,373838.2,' +
        '141532.01000000015,532809.75,0,193649.6699999998';
    const arr = str.split(',');
    const startindex = arr.findIndex(item => item.includes('('));
    const endindex = arr.findIndex(item => item.includes(')'));
    const indexStr = arr.slice(startindex, endindex + 1).join(',').replace(/"/g, '');
    arr.splice(startindex, endindex - startindex + 1, indexStr);
    expect(arr).toEqual(
        [
            '13.0',
            'stage',
            '(0, 1, 2, 3, 4, 5, 6, 7)',
            '294211.5200000002',
            '193649.6699999998',
            '180817.35000000015',
            '373838.2',
            '141532.01000000015',
            '532809.75',
            '0',
            '193649.6699999998',
        ],
    );
});

it('query step data', async function () {
    const response: [] = await globalDatabase.getStepIdList();
    const result = response.map((item: any) => item.stepId);
    console.log(response.map((item: any) => item.stepId));
    expect(result).toEqual(
        [ '12', '13', '14', '15', '16' ],
    );
});

it('query stage data', async function () {
    const response: [] = await globalDatabase.getStage('12');
    const result = response.map((item: any) => item.stageId);
    console.log(response.map((item: any) => item.stageId));
    expect(result).toEqual(
        ['(0, 1, 2, 3, 4, 5, 6, 7)'],
    );
});

it('query stage bubble time', async function () {
    const response: StageAndBubbleTimeResponse = { stageAndBubbleTimes: [] };
    response.stageAndBubbleTimes = await CLUSTER_DATABASE.getStageAndBubbleTime('12');
    expect(response).toEqual({
        stageAndBubbleTimes:
        [{
            bubbleTime: 0,
            stageId: '(0, 1, 2, 3, 4, 5, 6, 7)',
            stageTime: 5513799.3999,
        }],
    });
});

it('query stage bubble time', async function () {
    const response: StageAndBubbleTimeResponse = { stageAndBubbleTimes: [] };
    response.stageAndBubbleTimes = await CLUSTER_DATABASE.getRankAndBubbleTime('12', '(0, 1, 2, 3, 4, 5, 6, 7)');
    expect(response).toEqual({
        stageAndBubbleTimes: [
            {
                bubbleTime: 0,
                rankId: '0',
                stageTime: 4804145.1802,
            },
            {
                bubbleTime: 0,
                rankId: '1',
                stageTime: 4887810.1399,
            },
            {
                bubbleTime: 0,
                rankId: '2',
                stageTime: 5311810.2998,
            },
            {
                bubbleTime: 0,
                rankId: '3',
                stageTime: 4823466.99,
            },
            {
                bubbleTime: 0,
                rankId: '4',
                stageTime: 5513799.3999,
            },
            {
                bubbleTime: 0,
                rankId: '5',
                stageTime: 5005903.7402,
            },
            {
                bubbleTime: 0,
                rankId: '6',
                stageTime: 5246236.1399,
            },
            {
                bubbleTime: 0,
                rankId: '7',
                stageTime: 4907440.55,
            } ],
    });
});
