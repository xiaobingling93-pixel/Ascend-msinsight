import {
    EventRequest, ExtremumTimestamp,
    FlowDetailRequest,
    FlowDetailResponse,
    FlowResponse,
    LocationData, SimpleFlowDto,
    SimpleSlice,
    SliceDto,
    ThreadDetailRequest,
    ThreadDetailResponse,
    ThreadsRequest,
    ThreadsResponse,
} from './data';
import { InsightError } from '../utils/error';
import { getTrackId } from '../utils/common_util';
import { tableMap } from '../database/tableManager';
import { Table } from '../database/table';
import { Client } from '../types';

export const threadInfoHandler = async (request: ThreadDetailRequest, client: Client): Promise<ThreadDetailResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    let emptyFlag = true;
    const startTime = request.startTime + client.shadowSession.extremumTimestamp.minTimestamp;
    const trackId = getTrackId(request.tid, request.pid);
    const rows = await table.querySingleSliceByDepthTrackIdTime(request.depth, trackId, startTime) as SliceDto[];
    if (rows.length !== 1) {
        throw new InsightError(101, 'select slice error');
    }
    emptyFlag = false;
    let selfTime = rows[0].duration;
    const nextDepthResult = await table.queryDurationFromSliceByTimeRange(request.depth + 1,
        rows[0].timestamp + rows[0].duration, rows[0].timestamp, trackId) as Array<{duration: number}>;
    if (nextDepthResult.length === 0) {
        selfTime = 0;
    } else {
        nextDepthResult.forEach(row => {
            selfTime -= row.duration;
        });
    }
    return {
        emptyFlag,
        data: {
            selfTime,
            args: rows[0].args,
            title: rows[0].name,
            duration: rows[0].duration,
            cat: rows[0].cat ? rows[0].cat : '',
        },
    };
};

export const threadsInfoHandler = async (request: ThreadsRequest, client: Client): Promise<ThreadsResponse> => {
    const trackId = getTrackId(request.tid, request.pid);
    const startTime = request.startTime + client.shadowSession.extremumTimestamp.minTimestamp;
    const endTime = request.endTime + client.shadowSession.extremumTimestamp.minTimestamp;
    const table = tableMap.get(request.rankId) as Table;
    const extremumTimestamp = await table.queryExtremumTimeOfFirstDepth(trackId, startTime, endTime) as ExtremumTimestamp;
    const rows = await table.queryThreadsInfo(trackId, extremumTimestamp.minTimestamp, extremumTimestamp.maxTimestamp) as SimpleSlice[];
    let threadResponse: ThreadsResponse = { emptyFlag: false, data: [] };
    if (rows.length === 0) {
        threadResponse.emptyFlag = true;
        return threadResponse;
    }
    const selfTimeKeyValue: Record<string, number> = {};
    calculateSelfTime(rows, selfTimeKeyValue, startTime, endTime);
    const nRows = threadsInfoFilter(rows, startTime, endTime);
    threadResponse = reduceThread(nRows, selfTimeKeyValue);
    return threadResponse;
};

function threadsInfoFilter(rows: SimpleSlice[], startTime: number, endTime: number): SimpleSlice[] {
    const nRows = [] as SimpleSlice[];
    rows.forEach((row) => {
        if (row.timestamp <= endTime && row.endTime >= startTime) {
            nRows.push(row);
        }
    });
    return nRows;
};

function calculateSelfTime(rows: SimpleSlice[], selfTimeKeyValue: Record<string, number>, startTime: number, endTime: number): void {
    let i = 0;
    let j = 0;
    let tmpSelfTime = rows[0].duration;
    while (i < rows.length) {
        const rowI = rows[i];
        const rowJ = rows[j];
        // j滑完直接滑完所有i
        if (j === rows.length) {
            // 处理当前tmpSelfTime
            addData(selfTimeKeyValue, rows[i].name, tmpSelfTime);
            // 处理剩余元素
            while (++i < rows.length) {
                if (rows[i].timestamp <= endTime && rows[i].endTime >= startTime) {
                    addData(selfTimeKeyValue, rows[i].name, rows[i].duration);
                }
            }
            break;
        }
        // 层数相等 or 同一元素, j右移
        if (rowI.depth === rowJ.depth || i >= j) {
            j++;
            continue;
        }
        // rows[i]不属于框选范围内，跳过
        if (rows[i].timestamp > endTime || rows[i].endTime < startTime) {
            if (i + 1 === rows.length) { // i滑完结束
                break;
            }
            i++;
            tmpSelfTime = rows[i].duration;
            continue;
        }
        // j元素超出i元素覆盖范围，或者j右移到下一层, 记录i元素selfTime并i右移(隐式|| rowJ.timestamp < rowI.timestamp)
        if (rowJ.endTime > rowI.endTime || rowI.depth + 1 < rowJ.depth) {
            addData(selfTimeKeyValue, rowI.name, tmpSelfTime);
            if (i + 1 === rows.length) { // i滑完结束
                break;
            }
            i++;
            tmpSelfTime = rows[i].duration;
            continue;
        }
        // 符合要求的元素
        if (rowJ.timestamp >= rowI.timestamp && rowJ.endTime <= rowI.endTime) {
            tmpSelfTime -= rowJ.duration;
        }
        j++;
    }
}

function addData(selfTimeKeyValue: Record<string, number>, key: string, selfTime: number): void {
    if (selfTimeKeyValue[key]) {
        selfTimeKeyValue[key] += selfTime;
    } else {
        selfTimeKeyValue[key] = selfTime;
    }
}

function reduceThread(rows: SimpleSlice[], selfTimeKeyValue: {[key: string]: any}): ThreadsResponse {
    const tmp: ThreadsResponse = { emptyFlag: false, data: [] };
    return rows.reduce((acc, cur) => {
        const index = acc.data.findIndex((item) => item.title === cur.name);
        if (index === -1) {
            acc.data.push({
                title: cur.name,
                wallDuration: cur.duration,
                occurrences: 1,
                avgWallDuration: cur.duration,
                selfTime: selfTimeKeyValue[cur.name],
            });
        } else {
            acc.data[index].wallDuration += cur.duration;
            acc.data[index].occurrences += 1;
            acc.data[index].avgWallDuration = acc.data[index].wallDuration / acc.data[index].occurrences;
        }
        return acc;
    }, tmp);
}

export const flowNameHandler = async (request: EventRequest, client: Client): Promise<FlowResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const trackId = getTrackId(request.tid, request.pid);
    const startTime = request.startTime + client.shadowSession.extremumTimestamp.minTimestamp;
    const flowRows = await table.querySimpleFlowListByTrackIdTime(trackId, startTime) as SimpleFlowDto[];
    const type = flowRows[0].type;
    const response: FlowResponse = { flowDetail: [] };
    for (const row of flowRows) {
        const flowId = row.flowId;
        const toSliceDetail = await table.querySliceFlowList(flowId, type);
        response.flowDetail.push({
            title: row.name,
            tid: toSliceDetail[0]?.tid,
            pid: toSliceDetail[0]?.pid,
            timestamp: toSliceDetail[0].timestamp,
            depth: toSliceDetail[0]?.depth,
            flowId: row?.flowId,
        });
    }
    return response;
};

export const flowDetailHandler = async (request: FlowDetailRequest, client: Client): Promise<FlowDetailResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const rows = await table.queryLocationFlowByFlowId(request.flowId);
    if (rows.length !== 2) {
        throw new InsightError(101, 'select location error');
    }
    let fromLocation = {} as LocationData;
    let toLocation = {} as LocationData;
    for (const row of rows) {
        row.timestamp -= client.shadowSession.extremumTimestamp.minTimestamp;
        const tmpLocation = { pid: row.pid, tid: row.tid, timestamp: row.timestamp, depth: row.depth };
        if (row.type === 's') {
            fromLocation = tmpLocation;
        }
        if (row.type === 'f') {
            toLocation = tmpLocation;
        }
    }
    return {
        title: rows[0].name,
        cat: rows[0].cat,
        id: rows[0].flowId,
        from: fromLocation,
        to: toLocation,
    };
};
