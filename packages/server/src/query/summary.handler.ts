/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { StageAndBubbleTimeResponse, SummaryItemVO, SummaryRequest, SummaryStatisticsVO, SummaryVO } from './data';
import { CLUSTER_DATABASE, tableMap } from '../database/tableManager';
import { getLoggerByName } from '../logger/loggger_configure';
import { Client } from '../types';
import { isJsonStr } from '../utils/common_util';

const logger = getLoggerByName('summaryHandler', 'info');

export const summaryHandler = async (request: SummaryRequest, client: Client): Promise<Record<string, SummaryVO>> => {
    logger.info('request to summaryHandler');
    if (request.orderBy === '' || ![ 'computingTime', 'communicationNotOverLappedTime',
        'communicationOverLappedTime', 'freeTime' ].includes(request.orderBy)) {
        request.orderBy = 'computingTime';
    }
    const rows: SummaryItemVO[] = await CLUSTER_DATABASE.querySummaryData(request.orderBy, request.stepIdList, request.rankIdList);
    const extremumTimestamp = await client.shadowSession.extremumTimestamp;
    const baseInfo = await CLUSTER_DATABASE.queryBaseInfo();
    const result = {
        rankCount: rows.length,
        rankList: [],
        dataSize: 0,
        filePath: '',
        collectStartTime: new Date().getTime(),
        collectDuration: extremumTimestamp.maxTimestamp,
        stepNum: 0,
        stepList: [],
        summaryList: rows,
    };
    if (baseInfo.length > 0) {
        if (isJsonStr(baseInfo[0].ranks)) {
            result.rankList = JSON.parse(baseInfo[0].ranks);
        }
        result.dataSize = baseInfo[0].dataSize / (1024 * 1024);
        result.collectStartTime = baseInfo[0].collectStartTime;
        result.filePath = baseInfo[0].filePath;
        if (isJsonStr(baseInfo[0].steps)) {
            result.stepList = JSON.parse(baseInfo[0].steps);
            result.stepNum = result.stepList.length;
        }
    }
    return { result };
};

export const summaryStatisticHandler = async (request: {rankId: string; timeFlag: string; stepId: string}):
Promise<Record<string, SummaryStatisticsVO[]>> => {
    logger.info('request to summaryStatisticHandler', request.rankId, request.timeFlag);
    const table = tableMap.get(request.rankId);
    if (table === undefined) {
        logger.error('can not find this rank database,', request.rankId);
        return { result: [] };
    }
    let param: any[] = [];
    let stepCondition = '';
    if (request.stepId !== undefined && request.stepId !== '' && request.stepId !== 'ALL') {
        param.push(request.stepId);
        stepCondition = 'and step_id = ?';
    }
    const rows: SummaryStatisticsVO[] = [];
    if (request.timeFlag.includes('compute')) {
        const computeStatistics = await table.queryComputeStatisticsData(stepCondition, param);
        let total = 0;
        computeStatistics?.forEach((row: any) => {
            total = total + row.duration;
        });
        computeStatistics?.forEach((row: any) => {
            rows.push({
                overlapType: '',
                acceleratorCore: row.acceleratorCore,
                duration: row.duration,
                utilization: total > 0 ? row.duration / total : 0,
            });
        });
    } else {
        let timestampCondition = '';
        if (stepCondition !== '') {
            param = [];
            const stepDuration = await table.queryStepDuration('ProfilerStep#' + request.stepId);
            if (stepDuration !== undefined) {
                timestampCondition = ' and timestamp > ? and timestamp < ? ';
                param = [ stepDuration.timestamp, stepDuration.timestamp + stepDuration.duration ];
            }
        }
        const communicationStatistics = await table.queryCommunicationStatisticsData(timestampCondition, param);
        let overlapTime = 0; let communicationTime = 0;
        communicationStatistics?.forEach((row: any) => {
            row.overlapType === 'Communication' ? communicationTime = row.duration : overlapTime = row.duration;
        });
        rows.push({ overlapType: 'Communication(Overlapped)', acceleratorCore: '', duration: overlapTime, utilization: 0 });
        rows.push({ overlapType: 'Communication(Not Overlapped)', acceleratorCore: '', duration: communicationTime - overlapTime, utilization: 0 });
    }
    logger.info('end request to summaryStatisticHandler, rows:', rows);
    return { result: rows };
};

export const stepHandler = async (): Promise<{data: string[]}> => {
    const response: [] = await CLUSTER_DATABASE.getStepIdList();
    const result = response.map((item: any) => item.stepId);
    return { data: result };
};

export const stageHandler = async (request: {stepId: string}): Promise<{data: string[]}> => {
    const response: [] = await CLUSTER_DATABASE.getStage(request.stepId);
    const result = response.map((item: any) => item.stageId);
    return { data: result };
};

export const stageAndBubbleTimeHandler = async (request: {stepId: string}): Promise<StageAndBubbleTimeResponse> => {
    const response: StageAndBubbleTimeResponse = { stageAndBubbleTimes: [] };
    response.stageAndBubbleTimes = await CLUSTER_DATABASE.getStageAndBubbleTime(request.stepId);
    return response;
};

export const rankAndBubbleTimeHandler = async (request: {stepId: string; stageId: string}):
Promise<StageAndBubbleTimeResponse> => {
    const response: StageAndBubbleTimeResponse = { stageAndBubbleTimes: [] };
    response.stageAndBubbleTimes = await CLUSTER_DATABASE.getRankAndBubbleTime(request.stepId, request.stageId);
    return response;
};
