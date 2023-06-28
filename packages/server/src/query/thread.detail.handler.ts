import {
    EventRequest,
    FlowDao,
    FlowDetailRequest,
    FlowDetailResponse,
    FlowResponse,
    SliceDao,
    ThreadDetailRequest,
    ThreadDetailResponse,
    ThreadsRequest,
    ThreadsResponse,
} from './data';
import { InsightError } from '../utils/error';
import { Table } from '../database/table';
import { getTrackId } from '../utils/common_util';

const table: Table = new Table('./trace_view.db');
const sliceTable = 'slice';
const flowTable = 'flow';

export const threadInfoHandler = async (request: ThreadDetailRequest): Promise<ThreadDetailResponse> => {
    const depth = request.depth;
    const pid = request.pid;
    const tid = request.tid;
    const startTime = request.startTime;
    const trackId = getTrackId(tid, pid);
    const param = [ depth, trackId, startTime ];
    const sql: string = `SELECT * FROM ${sliceTable}
                            WHERE DEPTH = ? AND TRACK_ID = ? AND TIMESTAMP = ?`;
    let emptyFlag = true;
    const threadResponse: ThreadDetailResponse = { emptyFlag: true, data: [{ selfTime: 0, args: '', title: '', duration: 0 }] };
    const result = await table.selectData(sql, param);
    const rows = result as SliceDao[];
    if (rows.length !== 1) {
        throw new InsightError(101, 'select slice error');
    }
    emptyFlag = false;
    let selfTime = rows[0].duration;
    const endTime = rows[0].timestamp + rows[0].duration;
    const depthSql: string = `SELECT DURATION FROM ${sliceTable}
                            WHERE DEPTH = ? AND TIMESTAMP + DURATION &lt;= ? AND TIMESTAMP &gt;= ?`;
    const depthParams = [ depth, endTime, rows[0].timestamp ];
    const result2 = await table.selectData(depthSql, depthParams);
    const rows2 = result2 as number[];
    rows2.forEach(row => {
        selfTime -= row;
    });
    threadResponse.emptyFlag = emptyFlag;
    threadResponse.data[0].selfTime = selfTime;
    threadResponse.data[0].args = rows[0].args;
    threadResponse.data[0].title = rows[0].name;
    return threadResponse;
};

const emptyThreadsResponse = {
    emptyFlag: false,
    data: [{
        title: '',
        selfTime: 0,
        wallDuration: 0,
        avgWallDuration: 0,
        occurrences: 0,
    }],
};

export const threadsInfoHandler = async (request: ThreadsRequest): Promise<ThreadsResponse> => {
    const pid = request.pid;
    const tid = request.tid;
    const startTime = request.startTime;
    const endTime = request.endTime;
    const trackId = getTrackId(tid, pid);
    const param = [ trackId, startTime, endTime ];
    let selfTimeKeyValue: {[key: string]: any} = {};
    let threadResponse: ThreadsResponse = emptyThreadsResponse;
    const sql: string = `SELECT * FROM ${sliceTable} WHERE TRACK_ID = ? AND TIMESTAMP + DURATION &gt;= ? AND TIMESTAMP &lt;= ?`;
    table.selectData(sql, param).then(result => {
        const rows = result as SliceDao[];
        if (rows.length === 0) {
            threadResponse.emptyFlag = true;
            return;
        }
        rows.forEach(res => {
            let selfTime = res.duration;
            const endTime = rows[0].timestamp + rows[0].duration;
            const depth = res.depth;
            const depthSql: string = `SELECT DURATION FROM ${sliceTable}
                            WHERE DEPTH = ? AND TIMESTAMP + DURATION &lt;= ? AND TIMESTAMP &gt;= ?`;
            const depthParams = [ depth, endTime, rows[0].timestamp ];
            table.selectData(depthSql, depthParams).then(result => {
                const rows = result as number[];
                rows.forEach(row => {
                    selfTime -= row;
                });
            });
            selfTimeKeyValue = Object.assign({}, selfTimeKeyValue, { name: res.name, selfTime });
        });
        threadResponse = reduceThread(rows, selfTimeKeyValue);
    });
    return threadResponse;
};

function reduceThread(rows: SliceDao[], selfTimeKeyValue: {[key: string]: any}): ThreadsResponse {
    const tmp: ThreadsResponse = emptyThreadsResponse;
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
    const pid = request.pid;
    const tid = request.tid;
    const trackId = getTrackId(tid, pid);
    const startTime = request.startTime;
    const response: FlowResponse = { flowDetail: [{ title: '', timestamp: 0, track_id: 0 }] };
    const param = [ trackId, startTime ];
    const sql: string = `SELECT * FROM ${flowTable}
                            WHERE TRACK_ID = ? AND TIMESTAMP = ?`;
    const result = await table.selectData(sql, param);
    const rows = result as FlowDao[];
    rows.forEach(row => {
        response.flowDetail.push({ title: row.name, track_id: row.track_id, timestamp: row.timestamp });
    });
    return response;
};

export const flowDetailHandler = async (request: FlowDetailRequest): Promise<FlowDetailResponse> => {
    const pid = request.pid;
    const tid = request.tid;
    const trackId = getTrackId(tid, pid);
    const startTime = request.startTime;
    const title = request.title;
    const flowSql: string = `SELECT * FROM ${flowTable} WHERE TIMESTAMP = ? AND TRACK_ID = ? AND NAME = ?`;
    const flowParam = [ startTime, trackId, title ];
    const flowResult = await table.selectData(flowSql, flowParam) as FlowDao;
    const type = flowResult.type;
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
    const toSliceDetail = await table.selectData(toSql, toParam) as SliceDao;
    const fromParam = [ startTime, trackId, title, type, startTime, trackId, title, type ];
    const fromSql: string = `SELECT * FROM ${sliceTable} WHERE TIMESTAMP = ? AND TRACK_ID = ? AND NAME = ?)`;
    const fromSliceDetail = await table.selectData(fromSql, fromParam) as SliceDao;
    return {
        id: flowResult.flow_id,
        title: flowResult.name,
        cat: flowResult.cat,
        from: type === 's' ? fromSliceDetail : toSliceDetail,
        to: type === 's' ? toSliceDetail : fromSliceDetail,
    };
};
