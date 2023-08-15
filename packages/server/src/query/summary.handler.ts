/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { SummaryItemVO, SummaryRequest, SummaryStatisticsVO, SummaryVO } from './data';
import { CLUSTER_DATABASE, tableMap } from '../database/tableManager';
import { getLoggerByName } from '../logger/loggger_configure';
import { Client } from '../types';
import { isJsonStr } from '../utils/common_util';

const logger = getLoggerByName('summaryHandler', 'info');

export const summaryHandler = async (request: SummaryRequest, client: Client): Promise<Record<string, SummaryVO>> => {
    logger.info('request to summaryHandler');
    if (request.orderBy === '' || ![ 'computingTime', 'communicationNotOverLappedTime', 'communicationOverLappedTime', 'freeTime' ].includes(request.orderBy)) {
        request.orderBy = 'computingTime';
    }
    const rows: SummaryItemVO[] = await CLUSTER_DATABASE.querySummaryData(request.orderBy);
    const extremumTimestamp = await client.shadowSession.extremumTimestamp;
    const baseInfo = await CLUSTER_DATABASE.queryBaseInfo();
    const result = {
        rankCount: rows.length,
        rankList: [],
        dataSize: 0,
        filePath: '',
        collectStartTime: extremumTimestamp.minTimestamp,
        collectDuration: extremumTimestamp.maxTimestamp - extremumTimestamp.minTimestamp,
        stepNum: 0,
        stepList: [],
        summaryList: rows,
    };
    if (baseInfo.length > 0) {
        if (isJsonStr(baseInfo[0].ranks)) {
            result.rankList = JSON.parse(baseInfo[0].ranks);
        }
        result.dataSize = baseInfo[0].dataSize / (1024 * 1024);
        result.filePath = baseInfo[0].filePath;
        if (isJsonStr(baseInfo[0].steps)) {
            result.stepList = JSON.parse(baseInfo[0].steps);
            result.stepNum = result.stepList.length;
        }
    }
    return { result };
};

export const summaryStatisticHandler = async (request: {rankId: string; timeFlag: string}):
Promise<Record<string, SummaryStatisticsVO[]>> => {
    const table = tableMap.get(request.rankId);
    if (table === undefined) {
        logger.error('can not find this rank database,', request.rankId);
        return { result: [] };
    }
    const rows: SummaryStatisticsVO[] = [];
    if (request.timeFlag.includes('compute')) {
        const computeStatistics = await table.queryComputeStatisticsData();
        let total = 0;
        computeStatistics?.forEach((row: any) => {
            total = total + row.duration;
        });
        computeStatistics?.forEach((row: any) => {
            rows.push({ transportType: '', acceleratorCore: row.acceleratorCore, duration: row.duration, utilization: total > 0 ? row.duration / total : 0 });
        });
    } else {
        const communicationStatistics = await table.queryCommunicationStatisticsData();
        communicationStatistics?.forEach((row: any) => {
            rows.push({ transportType: row.transportType, acceleratorCore: '', duration: row.duration, utilization: 0 });
        });
    }
    return { result: rows };
};
