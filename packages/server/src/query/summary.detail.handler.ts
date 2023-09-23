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
    const opTrackId = await table.queryTrackId(threadName);
    const response: CommunicationDetailResponse = { totalNum: 0, communicationDetail: [] };
    const totalNum = await table.queryCommunicationTotalNum(opTrackId.track_id);
    response.totalNum = totalNum[0].nums;
    response.communicationDetail = await table.queryCommunicationDetailInfoBySort(request, opTrackId.track_id) as CommunicationDetail[];
    return response;
};

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
