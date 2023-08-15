/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { CommunicationDetailResponse, ComputeDetail, ComputeDetailResponse, SummaryDetailRequest } from './data';
import { tableMap } from '../database/tableManager';
import { Table } from '../database/table';
import { Client } from '../types';

export const computeDetailInfoHandler = async (request: SummaryDetailRequest, client: Client): Promise<ComputeDetailResponse> => {
    const timeFlag = request.timeFlag;
    const currentPage = request.currentPage;
    const pageSize = request.pageSize;
    const table = tableMap.get(request.rankId) as Table;
    const response: ComputeDetailResponse = { totalNum: 0, computeDetail: [] };
    response.computeDetail = await table.queryComputeDetailInfo(timeFlag, currentPage, pageSize, client) as ComputeDetail[];
    response.totalNum = await table.queryComputeTotalNum(timeFlag) as number;
    return response;
};

async function queryNotOverlapByTime(table: Table, rows: any[], notOverlapTrackId: number, trackId: number, client: Client, timeFlag: string): Promise<CommunicationDetailResponse> {
    const response: CommunicationDetailResponse = { totalNum: 0, communicationDetail: [] };
    response.totalNum = await table.queryCommunicationTotalNum(trackId, timeFlag);
    for (const row of rows) {
        let totalTime = 0;
        const timeStamp = row.startTime + client.shadowSession.extremumTimestamp.minTimestamp;
        const duration = row.duration;
        const res = await table.queryNotOverlapTime(table, notOverlapTrackId, timeStamp, duration);
        for (const re of res) {
            totalTime += re.duration;
        }
        response.communicationDetail.push({
            communicationKernel: row.name,
            startTime: row.startTime,
            totalDuration: row.duration,
            notOverlapDuration: totalTime,
            overlapDuration: duration - totalTime,
        });
    }
    return response;
}

export const communicationDetailInfoHandler = async (request: SummaryDetailRequest, client: Client): Promise<CommunicationDetailResponse> => {
    const timeFlag = request.timeFlag;
    const currentPage = request.currentPage;
    const pageSize = request.pageSize;
    const table = tableMap.get(request.rankId) as Table;
    const threadName = 'Communication OP';
    const notOverlap = 'communication_not_overlapped';
    const opTrackId = await table.queryTrackId(threadName);
    const notOverlapTrackId = await table.queryTrackId(notOverlap);
    const rows = await table.queryCommunicationDetailInfo(timeFlag, currentPage, pageSize, client, opTrackId.track_id);
    return queryNotOverlapByTime(table, rows, notOverlapTrackId.track_id, opTrackId.track_id, client, timeFlag);
};
