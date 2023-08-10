/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { SummaryItemVO, SummaryRequest, SummaryStatisticsVO, SummaryVO } from './data';
import { clusterDatabase } from '../database/tableManager';
import { Client } from '../types';

export const summaryHandler = async (request: SummaryRequest, client: Client): Promise<Record<string, SummaryVO>> => {
    const rows: SummaryItemVO[] = await clusterDatabase.querySummaryData();
    const result = {
        rankCount: rows.length,
        dataSize: 100,
        collectStartTime: 1000,
        filePath: 'filepath',
        collectDuration: 100,
        stepNum: 1,
        summaryList: rows,
    };
    return { result };
};

export const summaryStatisticHandler = async (request: {rankId: string; timeFlag: string}, client: Client):
Promise<Record<string, SummaryStatisticsVO[]>> => {
    const rows: SummaryStatisticsVO[] = [];
    if (request.timeFlag.includes('compute')) {
        rows.push({ transportType: '', acceleratorCore: 'AI_CPU', duration: 123, utilization: 0.5 });
        rows.push({ transportType: '', acceleratorCore: 'AI_CORE', duration: 122, utilization: 0.5 });
    } else {
        rows.push({ transportType: 'SDMA', acceleratorCore: '', duration: 123, utilization: 0 });
        rows.push({ transportType: 'REDUCE', acceleratorCore: '', duration: 122, utilization: 0 });
    }
    return { result: rows };
};
