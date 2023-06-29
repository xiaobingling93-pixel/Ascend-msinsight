import {
    EventRequest,
    FlowDao,
    FlowDetailRequest,
    FlowDetailResponse,
    FlowResponse,
    LocationData,
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
import { extremumTimestamp } from '../handlers/import';

const sliceTable = 'slice';
const flowTable = 'flow';
const threadTable = 'thread';

export const threadInfoHandler = async (request: ThreadDetailRequest): Promise<ThreadDetailResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const depth = request.depth;
    const pid = request.pid;
    const tid = request.tid;
    const startTime = request.startTime + extremumTimestamp.minTimestamp;
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

export const threadsInfoHandler = async (request: ThreadsRequest): Promise<ThreadsResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const pid = request.pid;
    const tid = request.tid;
    const startTime = request.startTime + extremumTimestamp.minTimestamp;
    const endTime = request.endTime + extremumTimestamp.minTimestamp;
    const trackId = getTrackId(tid, pid);
    const param = [ trackId, startTime, endTime ];
    const selfTimeKeyValue: Record<string, number> = {};
    let threadResponse: ThreadsResponse = { emptyFlag: false, data: [] };
    const sql: string = `SELECT * FROM ${sliceTable} WHERE TRACK_ID = ? AND TIMESTAMP + DURATION >= ? AND TIMESTAMP <= ?`;
    const rows = await table.selectData(sql, param) as SliceDao[];
    if (rows.length === 0) {
        threadResponse.emptyFlag = true;
        return threadResponse;
    }
    for (const res of rows) {
        let selfTime = res.duration;
        const endTime = res.timestamp + res.duration;
        const depth = res.depth;
        const depthSql: string = `SELECT DURATION FROM ${sliceTable}
                            WHERE DEPTH = ? AND TIMESTAMP + DURATION <= ? AND TIMESTAMP >= ? AND TRACK_ID = ?`;
        const depthParams = [ depth + 1, endTime, res.timestamp, trackId ];
        const nextDepthResult = await table.selectData(depthSql, depthParams) as Array<{duration: number}>;
        if (nextDepthResult.length === 0) {
            selfTime = 0;
        } else {
            nextDepthResult.forEach(row => {
                selfTime -= row.duration;
            });
        }
        addData(selfTimeKeyValue, res.name, selfTime);
    }
    threadResponse = reduceThread(rows, selfTimeKeyValue);
    return threadResponse;
};

function addData(selfTimeKeyValue: Record<string, number>, key: string, selfTime: number): void {
    if (selfTimeKeyValue[key]) {
        selfTimeKeyValue[key] += selfTime;
    } else {
        selfTimeKeyValue[key] = selfTime;
    }
}

function reduceThread(rows: SliceDao[], selfTimeKeyValue: {[key: string]: any}): ThreadsResponse {
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

export const flowNameHandler = async (request: EventRequest): Promise<FlowResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const pid = request.pid;
    const tid = request.tid;
    const trackId = getTrackId(tid, pid);
    const startTime = request.startTime + extremumTimestamp.minTimestamp;
    const response: FlowResponse = { flowDetail: [] };
    const param = [ trackId, startTime ];
    const sql: string = `SELECT * FROM ${flowTable}
                            WHERE TRACK_ID = ? AND TIMESTAMP = ?`;
    const result = await table.selectData(sql, param);
    const rows = result as FlowDao[];
    rows.forEach(row => {
        response.flowDetail.push({ title: row.name, trackId: row.track_id, timestamp: row.timestamp });
    });
    return response;
};

export const flowDetailHandler = async (request: FlowDetailRequest): Promise<FlowDetailResponse> => {
    const table = tableMap.get(request.rankId) as Table;
    const trackId = request.trackId;
    const startTime = request.startTime;
    const title = request.title;
    const flowSql: string = `SELECT * FROM ${flowTable} WHERE TIMESTAMP = ? AND TRACK_ID = ? AND NAME = ?`;
    const flowParam = [ startTime, trackId, title ];
    const flowResult = await table.selectData(flowSql, flowParam) as FlowDao[];
    const type = flowResult[0].type;
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
    const toLocation = await getLocationDataByTimeTrackId(table,
        [ toSliceDetail[0].timestamp, toSliceDetail[0].track_id ]);
    const fromLocation = await getLocationDataByTimeTrackId(table, [ startTime, trackId ]);
    return {
        id: flowResult[0].flow_id,
        title: flowResult[0].name,
        cat: flowResult[0].cat,
        from: type === 's' ? fromLocation[0] : toLocation[0],
        to: type === 's' ? toLocation[0] : fromLocation[0],
    };
};

async function getLocationDataByTimeTrackId(table: Table, param: number[]): Promise<LocationData[]> {
    const sql: string = `SELECT PID, TID, DEPTH, TIMESTAMP FROM ${threadTable} TH
                          LEFT JOIN ${sliceTable} SL ON SL.TRACK_ID = TH.TRACK_ID WHERE SL.TIMESTAMP = ? AND SL.TRACK_ID = ?`;
    return await table.selectData(sql, param);
}
