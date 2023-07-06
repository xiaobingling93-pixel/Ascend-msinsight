import {
    EventRequest,
    FlowDao,
    FlowDetailRequest,
    FlowDetailResponse,
    FlowResponse,
    LocationData, SimpleSlice,
    SliceDao,
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

const sliceTable = 'slice';
const flowTable = 'flow';
const threadTable = 'thread';

export const threadInfoHandler = async (request: ThreadDetailRequest, client: Client): Promise<ThreadDetailResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const depth = request.depth;
    const pid = request.pid;
    const tid = request.tid;
    const startTime = request.startTime + client.shadowSession.extremumTimestamp.minTimestamp;
    const trackId = getTrackId(tid, pid);
    const param = [ depth, trackId, startTime ];
    const sql: string = `SELECT * FROM ${sliceTable}
                            WHERE DEPTH = ? AND TRACK_ID = ? AND TIMESTAMP = ?`;
    let emptyFlag = true;
    const threadResponse: ThreadDetailResponse = { emptyFlag: true, data: { selfTime: 0, args: '', title: '', duration: 0, cat: '' } };
    const rows = await table.selectData(sql, param) as SliceDao[];
    if (rows.length !== 1) {
        throw new InsightError(101, 'select slice error');
    }
    emptyFlag = false;
    let selfTime = rows[0].duration;
    const endTime = rows[0].timestamp + rows[0].duration;
    const depthSql: string = `SELECT DURATION FROM ${sliceTable}
                            WHERE DEPTH = ? AND TIMESTAMP + DURATION <= ? AND TIMESTAMP >= ? AND TRACK_ID = ?`;
    const depthParams = [ depth + 1, endTime, rows[0].timestamp, trackId ];
    const nextDepthResult = await table.selectData(depthSql, depthParams) as Array<{duration: number}>;
    if (nextDepthResult.length === 0) {
        selfTime = 0;
    } else {
        nextDepthResult.forEach(row => {
            selfTime -= row.duration;
        });
    }
    threadResponse.emptyFlag = emptyFlag;
    threadResponse.data.selfTime = selfTime;
    threadResponse.data.args = rows[0].args;
    threadResponse.data.title = rows[0].name;
    threadResponse.data.duration = rows[0].duration;
    threadResponse.data.cat = rows[0].cat ? rows[0].cat : '';
    return threadResponse;
};

export const threadsInfoHandler = async (request: ThreadsRequest, client: Client): Promise<ThreadsResponse> => {
    const pid = request.pid;
    const tid = request.tid;
    const trackId = getTrackId(tid, pid);
    const startTime = request.startTime + client.shadowSession.extremumTimestamp.minTimestamp;
    const endTime = request.endTime + client.shadowSession.extremumTimestamp.minTimestamp;
    const param = [ trackId, endTime, startTime ];
    const sql: string = `SELECT timestamp, duration, timestamp + duration AS endTime, name, depth FROM ${sliceTable}
                         WHERE
                             TRACK_ID = ?
                           AND TIMESTAMP <= ?
                           AND TIMESTAMP + DURATION >= ?
                         ORDER BY
                             depth ASC,
                             timestamp ASC`;
    const table = tableMap.get(request.rankId) as Table;
    const rows = await table.selectData(sql, param) as SimpleSlice[];
    let threadResponse: ThreadsResponse = { emptyFlag: false, data: [] };
    if (rows.length === 0) {
        threadResponse.emptyFlag = true;
        return threadResponse;
    }
    const selfTimeKeyValue: Record<string, number> = {};
    calculateSelfTime(rows, selfTimeKeyValue);
    threadResponse = reduceThread(rows, selfTimeKeyValue);
    return threadResponse;
};

function calculateSelfTime(rows: SimpleSlice[], selfTimeKeyValue: Record<string, number>): void {
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
                addData(selfTimeKeyValue, rows[i].name, rows[i].duration);
            }
            break;
        }
        // 层数相等 or 同一元素, j右移
        if (rowI.depth === rowJ.depth || i === j) {
            j++;
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
    const pid = request.pid;
    const tid = request.tid;
    const trackId = getTrackId(tid, pid);
    const startTime = request.startTime + client.shadowSession.extremumTimestamp.minTimestamp;
    const flowSql: string = `SELECT * FROM ${flowTable} WHERE TIMESTAMP = ? AND TRACK_ID = ?`;
    const flowParam = [ startTime, trackId ];
    const flowRows = await table.selectData(flowSql, flowParam) as FlowDao[];
    const type = flowRows[0].type;
    const response: FlowResponse = { flowDetail: [] };
    for (const row of flowRows) {
        const title = row.name;
        const toParam = [ startTime, trackId, title, type, startTime, trackId, title, type ];
        const toSql: string = `SELECT * FROM ${sliceTable} WHERE TRACK_ID IN
                        (SELECT TRACK_ID FROM ${flowTable}
                            WHERE FLOW_ID IN
                                (SELECT FLOW_ID FROM ${flowTable} WHERE TIMESTAMP = ? AND TRACK_ID = ? AND NAME = ?)
                                AND TYPE <> ?)
                        AND TIMESTAMP IN
                            (SELECT TIMESTAMP FROM ${flowTable} WHERE FLOW_ID IN
                                (SELECT FLOW_ID FROM ${flowTable} WHERE TIMESTAMP = ? AND TRACK_ID = ? AND NAME = ?)
                                AND TYPE <> ?)`;
        const toSliceDetail = await table.selectData(toSql, toParam) as SliceDao[];
        const location =
            await getLocationDataByTimeTrackId(table, [ toSliceDetail[0].timestamp, toSliceDetail[0].track_id ]);
        response.flowDetail.push({
            title,
            tid: location[0].tid,
            pid: location[0].pid,
            timestamp: toSliceDetail[0].timestamp,
            depth: toSliceDetail[0].depth,
            flowId: row.flow_id,
        });
    }

    return response;
};

export const flowDetailHandler = async (request: FlowDetailRequest, client: Client): Promise<FlowDetailResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const flowId = request.flowId;
    const sql: string = `SELECT * FROM ${flowTable} WHERE FLOW_ID = ?`;
    const param = [flowId];
    const rows = await table.selectData(sql, param) as FlowDao[];
    let fromLocation = {} as LocationData[];
    let toLocation = {} as LocationData[];
    for (const row of rows) {
        const location = await getLocationDataByTimeTrackId(table, [ row.timestamp, row.track_id ]);
        location.forEach(row => {
            row.timestamp -= client.shadowSession.extremumTimestamp.minTimestamp;
        });
        if (row.type === 's') {
            fromLocation = location;
        }
        if (row.type === 'f') {
            toLocation = location;
        }
    }
    return {
        title: rows[0].name,
        cat: rows[0].cat,
        id: rows[0].flow_id,
        from: fromLocation[0],
        to: toLocation[0],
    };
};

async function getLocationDataByTimeTrackId(table: Table, param: number[]): Promise<LocationData[]> {
    const sql: string = `SELECT PID, TID, DEPTH, TIMESTAMP FROM ${threadTable} TH
                          LEFT JOIN ${sliceTable} SL ON SL.TRACK_ID = TH.TRACK_ID WHERE SL.TIMESTAMP = ? AND SL.TRACK_ID = ?`;
    return await table.selectData(sql, param);
}
