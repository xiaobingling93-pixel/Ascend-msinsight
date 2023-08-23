/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import {
    CommunicationDetailRequest,
    CommunicationDetailResponse,
    ComputeDetail, ComputeDetailRequest,
    ComputeDetailResponse,
} from './data';
import { tableMap } from '../database/tableManager';
import { Table } from '../database/table';
import { Client } from '../types';
import { KERNEL_DETAIL_TABLE } from '../common/sql_constant';

export const computeDetailInfoHandler = async (request: ComputeDetailRequest, client: Client): Promise<ComputeDetailResponse> => {
    const sql = genComputeSql(request);
    const timeFlag = request.timeFlag;
    const table = tableMap.get(request.rankId) as Table;
    const response: ComputeDetailResponse = { totalNum: 0, computeDetail: [] };
    response.computeDetail = await table.queryComputeDetailInfo(sql) as ComputeDetail[];
    const rows = await table.queryComputeTotalNum(timeFlag);
    response.totalNum = rows[0].nums;
    return response;
};

async function queryNotOverlapByTime(table: Table, rows: any[], notOverlapTrackId: number, trackId: number, client: Client): Promise<CommunicationDetailResponse> {
    const response: CommunicationDetailResponse = { totalNum: 0, communicationDetail: [] };
    const res = await table.queryCommunicationTotalNum(trackId);
    response.totalNum = res[0].nums;
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

export const communicationDetailInfoHandler = async (request: CommunicationDetailRequest, client: Client): Promise<CommunicationDetailResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const threadName = 'Group 0 Communication';
    const notOverlap = 'Communication(Not Overlapped)';
    const opTrackId = await table.queryTrackId(threadName);
    const notOverlapTrackId = await table.queryTrackId(notOverlap);
    const rows = await table.queryCommunicationDetailInfo(request, client, opTrackId.track_id);
    return queryNotOverlapByTime(table, rows, notOverlapTrackId.track_id, opTrackId.track_id, client);
};

function genComputeSql(request: ComputeDetailRequest): string {
    const orderList = request.orderList;
    const offset = (request.currentPage - 1) * request.pageSize;
    const ascend = request.sortBy === 'ascend' ? 'ASC' : 'DESC';
    let sql: string = '';
    if (orderList.length === 0) {
        sql = `SELECT name, type, start_time as startTime, duration, wait_time, block_dim, input_shapes, input_data_types,
                    input_formats, output_shapes, output_data_types, output_formats
               FROM ${KERNEL_DETAIL_TABLE}
               WHERE accelerator_core = '${request.timeFlag}'  LIMIT ${request.pageSize} offset ${offset}`;
    } else {
        sql = `SELECT name, type, start_time as startTime, duration, wait_time, block_dim, input_shapes, input_data_types,
                    input_formats, output_shapes, output_data_types, output_formats
               FROM ${KERNEL_DETAIL_TABLE}
               WHERE accelerator_core = '${request.timeFlag}'  order by "${orderList}" ${ascend} LIMIT ${request.pageSize} offset ${offset}`;
    }
    return sql;
}
