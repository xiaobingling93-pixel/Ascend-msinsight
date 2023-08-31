/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import {
    CommunicationDetail,
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

export const communicationDetailInfoHandler = async (request: CommunicationDetailRequest, client: Client): Promise<CommunicationDetailResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const threadName = 'Group 0 Communication';
    const notOverlap = 'Communication(Not Overlapped)';
    const opTrackId = await table.queryTrackId(threadName);
    const notOverlapTrackId = await table.queryTrackId(notOverlap);
    const response: CommunicationDetailResponse = { totalNum: 0, communicationDetail: [] };
    const totalNum = await table.queryCommunicationTotalNum(opTrackId.track_id);
    response.totalNum = totalNum[0].nums;
    const rows = await table.queryCommunicationDetailInfo(client, opTrackId.track_id);
    response.communicationDetail = await queryNotOverlapByTime(request, response, table, rows, notOverlapTrackId.track_id, client);
    return response;
};

async function queryNotOverlapByTime(request: CommunicationDetailRequest, response: CommunicationDetailResponse, table: Table, rows: any[], notOverlapTrackId: number, client: Client): Promise<CommunicationDetail[]> {
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
    const orderBy = request.orderBy;
    const pageSize = request.pageSize;
    const currentPage = request.currentPage;
    const ascend = request.order;
    sortByRequest(response.communicationDetail, orderBy, ascend);
    const length = response.communicationDetail.length;
    if ((currentPage * pageSize - 1) > (length - 1)) {
        return response.communicationDetail.slice((currentPage - 1) * pageSize, length);
    }
    return response.communicationDetail.slice((currentPage - 1) * pageSize, currentPage * pageSize - 1);
}

function genComputeSql(request: ComputeDetailRequest): string {
    const orderList = request.orderBy;
    const offset = (request.currentPage - 1) * request.pageSize;
    const ascend = request.order === 'ascend' ? 'ASC' : 'DESC';
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

function sortByRequest(communicationDetail: CommunicationDetail[], orderBy: string, ascend: string): void {
    switch (orderBy) {
        case 'startTime': {
            if (ascend === 'ascend') {
                communicationDetail.sort((a, b) => a.startTime - b.startTime);
            } else {
                communicationDetail.sort((a, b) => b.startTime - a.startTime);
            }
            break;
        }
        case 'totalDuration': {
            if (ascend === 'ascend') {
                communicationDetail.sort((a, b) => a.totalDuration - b.totalDuration);
            } else {
                communicationDetail.sort((a, b) => b.totalDuration - a.totalDuration);
            }
            break;
        }
        case 'overlapDuration': {
            if (ascend === 'ascend') {
                communicationDetail.sort((a, b) => a.overlapDuration - b.overlapDuration);
            } else {
                communicationDetail.sort((a, b) => b.overlapDuration - a.overlapDuration);
            }
            break;
        }
        case 'notOverlapDuration': {
            if (ascend === 'ascend') {
                communicationDetail.sort((a, b) => a.notOverlapDuration - b.notOverlapDuration);
            } else {
                communicationDetail.sort((a, b) => b.notOverlapDuration - a.notOverlapDuration);
            }
            break;
        }
        default: {
            if (ascend === 'ascend') {
                communicationDetail.sort((a, b) => a.communicationKernel.localeCompare(b.communicationKernel));
            } else {
                communicationDetail.sort((a, b) => b.communicationKernel.localeCompare(a.communicationKernel));
            }
            break;
        }
    }
}
