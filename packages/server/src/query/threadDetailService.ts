import { SliceDao, ThreadDetailRequest, ThreadDetailResponse } from './data';
import { Table } from '../parse/table';
import { getTrackId } from '../common/common';

const table: Table = new Table('./trace_view.db');
const sliceTable = 'slice';

export function queryThreadInfo(request: ThreadDetailRequest): ThreadDetailResponse {
    const depth = request.depth;
    const pid = request.pid;
    const tid = request.tid;
    const startTime = request.startTime;
    const trackId = getTrackId(tid, pid);
    const param = [ depth, trackId, startTime ];
    const sql: string = `SELECT NAME, DURATION, TIMESTAMP, ARGS FROM ${sliceTable}
                            WHERE DEPTH = ? AND TRACK_ID = ? AND TIMESTAMP = ?`;
    let emptyFlag = false;
    let selfTime = 0;
    const threadResponse: ThreadDetailResponse = { emptyFlag: false, data: { selfTime: 0, args: '', title: '', duration: 0 } };
    table.selectData(sql, param).then(result => {
        const rows = result as SliceDao[];
        if (rows.length === 0) {
            emptyFlag = true;
        }
        if (rows.length !== 1) {
            throw new Error('select error');
        }
        selfTime = rows[0].duration;
        const endTime = rows[0].timestamp + rows[0].duration;
        const depthSql: string = `SELECT DURATION FROM ${sliceTable}
                            WHERE DEPTH = ? AND TIMESTAMP + DURATION <= ? AND TIMESTAMP >= ?`;
        const depthParams = [ depth, endTime, rows[0].timestamp ];
        table.selectData(depthSql, depthParams).then(result => {
            const rows = result as SliceDao[];
            rows.forEach(row => {
                selfTime -= row.duration;
            });
        });
        threadResponse.emptyFlag = emptyFlag;
        threadResponse.data.selfTime = selfTime;
        threadResponse.data.args = rows[0].args;
        threadResponse.data.title = rows[0].name;
    });

    return threadResponse;
}
