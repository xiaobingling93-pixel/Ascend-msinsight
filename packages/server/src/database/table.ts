import * as sqlite from 'sqlite3';
import { RowThreadTrace, ThreadTrace } from '../query/thread.trace.handler';
import { Client } from '../types';
import { getLoggerByName } from '../logger/loggger_configure';
import { CREATE_KERNEL_DETAIL_TABLE_SQL, KERNEL_DETAIL_TABLE } from '../common/sql_constant';
import { KernelDetailEntity } from '../query/entity';
import { CommunicationDetailRequest } from '../query/data';

const logger = getLoggerByName('table', 'info');

export class Table {
    private readonly db: sqlite.Database;
    private readonly sliceTable = 'slice';
    private readonly threadTable = 'thread';
    private readonly processTable = 'process';
    private readonly flowTable = 'flow';
    private readonly idIndex = 'id_index';
    private readonly trackIdTimeIndex = 'track_id_time_index';
    private sliceDataCaches: any[] = [];
    private flowDataCaches: any[] = [];
    private kernelDetailCaches: any[] = [];
    private readonly maxCachesSize = 1000;
    private readonly maxKernelCacheSize = 100;
    count = 0;
    private sliceStat: sqlite.Statement | undefined;
    private flowStat: sqlite.Statement | undefined;
    private kernelDetailStat: sqlite.Statement | undefined;
    private readonly _dbPath: string;

    constructor(dbPath: string) {
        this.db = new sqlite.Database(dbPath, sqlite.OPEN_READWRITE | sqlite.OPEN_CREATE | sqlite.OPEN_SHAREDCACHE, (err) => {
            if (err !== null) {
                logger.error(`Connect to database error.${err.message}`);
            }
            logger.info('Connect to database.');
        });
        this._dbPath = dbPath;
    }

    async setConfig(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.db.run('PRAGMA synchronous = OFF')
                .run('PRAGMA journal_mode = MEMORY', (err) => {
                    if (err) {
                        logger.error(err.message);
                    }
                    logger.info('set config');
                    resolve();
                });
        });
    }

    initStat(): void {
        if (this.sliceStat === undefined) {
            const valueParams = '(round(? * 1000),round(? * 1000),?,?,?,?)';
            const placeholders: string = (valueParams + ',').repeat(this.maxCachesSize - 1).concat(valueParams);
            const sql: string = `INSERT INTO ${this.sliceTable} (timestamp, duration, name, track_id, cat, args) VALUES ${placeholders}`;
            this.sliceStat = this.db.prepare(sql);
        }
        if (this.flowStat === undefined) {
            const valueParams = '(?,?,?,round(? * 1000),?,?)';
            const placeholders: string = (valueParams + ',').repeat(this.maxCachesSize - 1).concat((valueParams));
            const sql: string = `INSERT INTO ${this.flowTable} (flow_id, name, track_id, timestamp, cat, type) VALUES ${placeholders}`;
            this.flowStat = this.db.prepare(sql);
        }
    }

    async createTable(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.db.serialize(() => {
                this.db.run(`DROP TABLE IF EXISTS ${this.sliceTable}`)
                    .run(`CREATE TABLE ${this.sliceTable} (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, duration INTEGER, name TEXT, depth INTEGER, track_id INTEGER, cat TEXT, args TEXT)`)
                    .run(`DROP TABLE IF EXISTS ${this.threadTable}`)
                    .run(`CREATE TABLE ${this.threadTable} (track_id INTEGER PRIMARY KEY, tid INTEGER, pid TEXT, thread_name TEXT, thread_sort_index INTEGER)`)
                    .run(`DROP TABLE IF EXISTS ${this.processTable}`)
                    .run(`CREATE TABLE ${this.processTable} (pid TEXT PRIMARY KEY, process_name TEXT, label TEXT, process_sort_index INTEGER)`)
                    .run(`DROP TABLE IF EXISTS ${this.flowTable}`)
                    .run(`CREATE TABLE ${this.flowTable} (id INTEGER PRIMARY KEY AUTOINCREMENT, flow_id TEXT, name TEXT, cat TEXT, track_id INTEGER, timestamp INTEGER, type TEXT)`)
                    .run(`DROP TABLE IF EXISTS ${KERNEL_DETAIL_TABLE}`)
                    .run(CREATE_KERNEL_DETAIL_TABLE_SQL)
                    .run(`DROP INDEX IF EXISTS ${this.idIndex}`)
                    .run(`DROP INDEX IF EXISTS ${this.trackIdTimeIndex}`)
                    .run('PRAGMA synchronous = OFF')
                    .run('PRAGMA journal_mode = MEMORY', (err) => {
                        if (err) {
                            logger.error(err.message);
                        }
                        logger.info('Create table end.');
                        resolve();
                    });
            });
        });
    }

    async close(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.sliceStat?.finalize(() => {
                logger.info('slice statement finalize close.');
            });
            this.flowStat?.finalize(() => {
                logger.info('flow statement finalize close.');
            });
            this.kernelDetailStat?.finalize(() => {
                logger.info('kernelDetail statement finalize close.');
            });
            this.db.close((err) => {
                if (err !== null) {
                    logger.error(err.message);
                    reject(err);
                }
                logger.info('Close the database connection.');
                resolve();
            });
        });
    }

    async insertSliceList(dataList: Array<{ name: string; ts: number; dur: number; track_id: number; cat: string; args: any }>): Promise<void> {
        return new Promise((resolve, reject) => {
            const paramsList: any[] = [];
            dataList.forEach((data) => { paramsList.push(data.ts, data.dur, data.name, data.track_id, data.cat, JSON.stringify(data.args)); });
            if (dataList.length === this.maxCachesSize) {
                if (this.sliceStat === undefined) {
                    this.initStat();
                }
                this.sliceStat?.reset().run(paramsList, (err) => {
                    if (err) {
                        logger.error(err.message);
                        reject(err);
                    }
                    ++this.count;
                    if (this.count % 100 === 0) {
                        logger.info('insertSliceList count.', this.count * dataList.length);
                    }
                    resolve();
                });
            } else {
                const placeholders: string = dataList.map(() => '(round(? * 1000),round(? * 1000),?,?,?,?)').join(',');
                const sql: string = `INSERT INTO ${this.sliceTable} (timestamp, duration, name, track_id, cat, args) VALUES ${placeholders}`;
                this.db.run(sql, paramsList, (err) => {
                    if (err !== null) {
                        logger.error(err.message);
                        reject(err);
                    }
                    ++this.count;
                    if (this.count % 100 === 0) {
                        logger.info('insertSliceList count.', this.count * dataList.length);
                    }
                    resolve();
                });
            }
        });
    }

    async insertSlice(data: { name: string; ts: number; dur: number; track_id: number; cat: string; args: any }): Promise<void> {
        return new Promise((resolve, reject) => {
            this.sliceDataCaches.push(data);
            if (this.sliceDataCaches.length >= this.maxCachesSize) {
                this.insertSliceList(this.sliceDataCaches).finally(() => {
                    this.sliceDataCaches = [];
                    resolve();
                });
            } else {
                resolve();
            }
        });
    }

    insertKernelDetailList(dataList: KernelDetailEntity[]): void {
        const paramsList: any[] = [];
        dataList.forEach((data) => {
            paramsList.push(data.stepId, data.name, data.type, data.acceleratorCore, data.startTime, data.duration,
                data.waitTime, data.blockDim, data.inputShapes, data.inputDataTypes, data.inputFormats,
                data.outputShapes, data.outputDataTypes, data.outputFormats);
        });
        const valueParams = '(?,?,?,?,?,?,?,?,?,?,?,?,?,?)';
        const columns = 'step_id,name,type,accelerator_core,start_time,duration,wait_time,' +
            'block_dim,input_shapes,input_data_types,input_formats,output_shapes,output_data_types,output_formats';
        if (dataList.length === this.maxKernelCacheSize) {
            logger.info('start execute insert into kernel detail table prepare statement ');
            if (this.kernelDetailStat === undefined) {
                const placeholders: string = (valueParams + ',').repeat(this.maxKernelCacheSize - 1).concat(valueParams);
                const sql: string = `INSERT INTO ${KERNEL_DETAIL_TABLE} (${columns})VALUES ${placeholders}`;
                this.kernelDetailStat = this.db.prepare(sql);
            }
            this.kernelDetailStat?.reset().run(paramsList, (err) => {
                if (err) {
                    logger.error('execute insert into kernel detail table is failed:{}', err.message);
                }
            });
        } else {
            logger.info('start run insert into kernel detail table sql,data size : ', dataList.length);
            const placeholders: string = dataList.map(() => valueParams).join(',');
            const sql: string = `INSERT INTO ${KERNEL_DETAIL_TABLE} (${columns})VALUES ${placeholders}`;
            this.db.run(sql, paramsList, (err) => {
                if (err !== null) {
                    logger.error('execute insert into kernel detail table is failed:{}', err.message);
                }
            });
        }
    }

    insertKernelDetail(data: KernelDetailEntity): void {
        this.kernelDetailCaches.push(data);
        if (this.kernelDetailCaches.length >= this.maxKernelCacheSize) {
            this.insertKernelDetailList(this.kernelDetailCaches);
            this.kernelDetailCaches = [];
        }
    }

    updateProcessName(pid: string, name: string): void {
        const sql: string = `INSERT INTO ${this.processTable} (pid, process_name) VALUES (?, ?)
                            ON CONFLICT (pid) DO UPDATE SET process_name = excluded.process_name`;
        this.db.run(sql, [ pid, name ], (err) => {
            if (err !== null) {
                logger.error(err.message);
            }
        });
    }

    updateProcessLabel(pid: string, label: string): void {
        const sql: string = `INSERT INTO ${this.processTable} (pid, label) VALUES (?, ?)
                            ON CONFLICT (pid) DO UPDATE SET label = excluded.label;`;
        this.db.run(sql, [ pid, label ], (err) => {
            if (err !== null) {
                logger.error(err.message);
            }
        });
    }

    updateProcessSortIndex(pid: string, sortIndex: number): void {
        const sql: string = `INSERT INTO ${this.processTable} (pid, process_sort_index) VALUES (?, ?)
                            ON CONFLICT (pid) DO UPDATE SET process_sort_index = excluded.process_sort_index;`;
        this.db.run(sql, [ pid, sortIndex ], (err) => {
            if (err !== null) {
                logger.error(err.message);
            }
        });
    }

    updateThreadName(trackId: number, tid: number, pid: string, threadName: string): void {
        const sql: string = `INSERT INTO ${this.threadTable} (track_id, tid, pid, thread_name) VALUES (?, ?, ?, ?)
                            ON CONFLICT (track_id) DO UPDATE
                            SET tid = excluded.tid, pid = excluded.pid, thread_name = excluded.thread_name`;
        this.db.run(sql, [ trackId, tid, pid, threadName ], (err) => {
            if (err !== null) {
                logger.error(err.message);
            }
        });
    }

    updateThreadSortIndex(trackId: number, sortIndex: number): void {
        const sql: string = `INSERT INTO ${this.threadTable} (track_id, thread_sort_index) VALUES (?, ?)
                            ON CONFLICT (track_id) DO UPDATE SET thread_sort_index = excluded.thread_sort_index`;
        this.db.run(sql, [ trackId, sortIndex ], (err) => {
            if (err !== null) {
                logger.error(err.message);
            }
        });
    }

    get dbPath(): string {
        return this._dbPath;
    }

    async insertFlowList(dataList: Array<{ name: string; cat: string; id: string; track_id: number; ts: number; ph: string }>): Promise<void> {
        return new Promise((resolve, reject) => {
            const paramsList: any[] = [];
            dataList.forEach((data) => { paramsList.push(data.id, data.name, data.track_id, data.ts, data.cat, data.ph); });
            if (dataList.length === this.maxCachesSize) {
                if (this.flowStat === undefined) {
                    this.initStat();
                }
                this.flowStat?.reset().run(paramsList, (err) => {
                    if (err) {
                        logger.error(err.message);
                        reject(err);
                    }
                    resolve();
                });
            } else {
                const placeholders: string = dataList.map(() => '(?,?,?,round(? * 1000),?,?)').join(',');
                const sql: string = `INSERT INTO ${this.flowTable} (flow_id, name, track_id, timestamp, cat, type) VALUES ${placeholders}`;
                this.db.run(sql, paramsList, (err) => {
                    if (err !== null) {
                        logger.error(err.message);
                        reject(err);
                    }
                    resolve();
                });
            }
        });
    }

    async insertFlow(data: { name: string; cat: string; id: string; track_id: number; ts: number; ph: string }): Promise<void> {
        return new Promise((resolve, reject) => {
            this.flowDataCaches.push(data);
            if (this.flowDataCaches.length === this.maxCachesSize) {
                this.insertFlowList(this.flowDataCaches).finally(() => {
                    this.flowDataCaches = [];
                    resolve();
                });
            } else {
                resolve();
            }
        });
    }

    async beginTransaction(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.db.run('BEGIN', (err) => {
                if (err) {
                    logger.error(err.message);
                    reject(err);
                }
                resolve();
            });
        });
    }

    async endTransaction(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.db.run('COMMIT', (err) => {
                if (err) {
                    logger.error(err.message);
                    reject(err);
                }
                resolve();
            });
        });
    }

    async commitData(): Promise<void> {
        if (this.sliceDataCaches.length > 0) {
            await this.insertSliceList(this.sliceDataCaches);
            this.sliceDataCaches = [];
        }
        if (this.flowDataCaches.length > 0) {
            await this.insertFlowList(this.flowDataCaches);
            this.flowDataCaches = [];
        }
    }

    saveLastKernelData(): void {
        if (this.kernelDetailCaches.length > 0) {
            this.insertKernelDetailList(this.kernelDetailCaches);
            this.kernelDetailCaches = [];
        }
    }

    async selectExtremumTimestamp(): Promise<any> {
        const sql: string = `select min(timestamp) as minTimestamp, max(timestamp) as maxTimestamp from ${this.sliceTable};`;
        return new Promise((resolve, reject) => {
            this.db.get(sql, (err, row) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(row);
                }
            });
        });
    }

    async selectUnitsMetadata(): Promise<any> {
        const sql: string = `SELECT
                                 pt.pid,
                                 pt.process_name AS processName,
                                 pt.label,
                                 pt.tid,
                                 pt.thread_name AS threadName,
                                 max( depth ) + 1 AS maxDepth
                             FROM
                                 (
                                     SELECT
                                         p.pid,
                                         p.process_name,
                                         p.label,
                                         p.process_sort_index,
                                         t.tid,
                                         t.thread_name,
                                         t.track_id,
                                         t.thread_sort_index
                                     FROM
                                         ${this.processTable} p
                                             LEFT JOIN ${this.threadTable} t ON p.pid = t.pid
                                 ) AS pt
                                     LEFT JOIN ${this.sliceTable} s ON pt.track_id = s.track_id
                             WHERE pt.process_name is not null
                             GROUP BY
                                 s.track_id
                             ORDER BY
                                 pt.process_sort_index ASC,
                                 pt.thread_sort_index ASC;`;
        return new Promise((resolve, reject) => {
            this.db.all(sql, function (err, rows) {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async creatIndex(): Promise<void> {
        return new Promise((resolve, reject) => {
            const start = new Date().getTime();
            this.db.serialize(() => {
                this.db
                    .run(`CREATE INDEX ${this.idIndex} ON ${this.sliceTable} (id)`)
                    .run(`CREATE INDEX ${this.trackIdTimeIndex} ON ${this.sliceTable} (track_id, timestamp)`, (err) => {
                        if (err) {
                            logger.error(err.message);
                            reject(err);
                        }
                        logger.info('create id index end.', new Date().getTime() - start);
                        resolve();
                    });
            });
        });
    }

    async getTrackIdList(): Promise<number[]> {
        return new Promise((resolve, reject) => {
            this.db.all(`SELECT track_id From ${this.threadTable}`, (err, rows: any[]) => {
                if (err) {
                    logger.error(err.message);
                    reject(err);
                }
                const tracks = [];
                if (rows !== undefined && rows.length > 0) {
                    for (const row of rows) {
                        tracks.push(row.track_id);
                    }
                }
                resolve(tracks);
            });
        });
    }

    async updateDepth(): Promise<void> {
        const start = new Date().getTime();
        const trackIdList = await this.getTrackIdList();
        for (const trackId of trackIdList) {
            await this.updateOneTrackDepth(trackId);
        }
        logger.info('update depth end. time:', new Date().getTime() - start);
    }

    async updateOneTrackDepth(trackId: number): Promise<void> {
        return new Promise((resolve, reject) => {
            const start = new Date().getTime();
            const maxParams = 10000;
            this.db.serialize(() => {
                this.db.run('BEGIN')
                    .all(`SELECT id, timestamp, duration, track_id FROM ${this.sliceTable} WHERE track_id = ? ORDER BY timestamp`,
                        [trackId], (err, rows: any[]) => {
                            if (err) {
                                logger.error(err.message);
                                reject(err);
                            }
                            const depthMap = this.getDepth(rows);
                            depthMap.forEach((idList, depth) => {
                                while (idList.length > 0) {
                                    const paramList = idList.splice(0, maxParams);
                                    const placeholders: string = paramList.map((id) => `${id}`).join(',');
                                    const sql: string = `UPDATE ${this.sliceTable} set depth = ${depth} WHERE id in (${placeholders})`;
                                    this.db.run(sql);
                                }
                            });
                        })
                    .run('COMMIT', (err) => {
                        if (err) {
                            logger.error(err.message);
                            reject(err);
                        }
                        logger.info(`trackId ${trackId} update depth end. time:${new Date().getTime() - start}`);
                        resolve();
                    });
            });
        });
    }

    getDepth(rows: any[]): Map<number, number[]> {
        const depthCache: number[] = [];
        const depthMap = new Map<number, number[]>();
        for (const row of rows) {
            let depth = -1;
            for (let i = 0; i < depthCache.length; ++i) {
                if (row.timestamp > depthCache[i]) {
                    depthCache[i] = row.timestamp + row.duration;
                    depth = i;
                    break;
                }
            }
            if (depth < 0) {
                depth = depthCache.length;
                depthCache.push(row.timestamp + row.duration);
                depthMap.set(depth, []);
            }
            depthMap.get(depth)?.push(row.id);
        }
        return depthMap;
    }

    async queryExtremumTimeOfFirstDepth(trackId: number, startTime: number, endTime: number): Promise<any> {
        const sql: string = `SELECT min(timestamp) as minTimestamp, max(timestamp + duration) AS maxTimestamp
                             FROM ${this.sliceTable}
                             WHERE
                                 TRACK_ID = ?
                               AND TIMESTAMP <= ?
                               AND TIMESTAMP + DURATION >= ?
                               AND DEPTH = 0`;
        return new Promise((resolve, reject) => {
            this.db.get(sql, [ trackId, endTime, startTime ], (err, row) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(row);
                }
            });
        });
    }

    async queryTrackId(threadName: String): Promise<any> {
        const sql: string = `SELECT track_id
                             FROM ${this.threadTable}
                             WHERE thread_name = ?`;
        return new Promise((resolve, reject) => {
            this.db.get(sql, [threadName], (err, row) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(row);
                }
            });
        });
    }

    async queryThreadsInfo(trackId: number, minTimestamp: number, maxTimestamp: number): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT timestamp, duration, timestamp + duration AS endTime, name, depth FROM ${this.sliceTable}
                         WHERE
                             TRACK_ID = ?
                           AND TIMESTAMP <= ?
                           AND TIMESTAMP + DURATION >= ?
                         ORDER BY
                             depth ASC,
                             timestamp ASC`;
            this.db.all(sql, [ trackId, maxTimestamp, minTimestamp ], (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryCommunicationDetailInfo(request: CommunicationDetailRequest, client: Client, trackId: number): Promise<any> {
        const map = new Map([
            [ 'communicationKernel', 'name' ],
            [ 'startTime', 'startTime' ],
            [ 'totalDuration', 'duration' ],
        ]);
        const orderList = request.orderBy;
        const offset = (request.currentPage - 1) * request.pageSize;
        const ascend = request.order === 'ascend' ? 'ASC' : 'DESC';
        let sql: string = '';
        return new Promise((resolve, reject) => {
            if (orderList.length === 0) {
                sql = `SELECT name, timestamp -${client.shadowSession.extremumTimestamp.minTimestamp} as startTime, duration
               FROM ${this.sliceTable}
               WHERE track_id = ${trackId} LIMIT ${request.pageSize} offset ${offset}`;
            } else {
                sql = `SELECT name, timestamp -${client.shadowSession.extremumTimestamp.minTimestamp} as startTime, duration
               FROM ${this.sliceTable}
               WHERE track_id = ${trackId}  order by "${map.get(orderList)}" ${ascend} LIMIT ${request.pageSize} offset ${offset}`;
            }
            this.db.all(sql, (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryComputeDetailInfo(sql: string): Promise<any> {
        return new Promise((resolve, reject) => {
            this.db.all(sql, async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryComputeTotalNum(timeFlag: string): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT
                                    count(*) as nums
                                    FROM ${KERNEL_DETAIL_TABLE}
                                    WHERE accelerator_core = ?`;
            this.db.all(sql, [timeFlag], async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryCommunicationTotalNum(trackId: number): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT count(*) as nums
                                 FROM ${this.sliceTable}
                                 WHERE
                                     track_id = ?`;
            this.db.all(sql, [trackId], (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryThreadTraceList(client: Client, threadId: number, trackId: number, startTime: number, endTime: number): Promise<ThreadTrace[][]> {
        return new Promise((resolve, reject) => {
            const rowDatas: ThreadTrace[][] = [];
            this.db.all(`SELECT id, timestamp - ${client.shadowSession.extremumTimestamp.minTimestamp} as start_time, duration, name, depth, track_id
            FROM ${this.sliceTable}
            WHERE track_id = ${trackId}
            AND start_time >= ${startTime} AND start_time <= ${endTime}
            GROUP BY depth, id
            ORDER BY start_time;`,
            async (err, rows: any) => {
                if (err) {
                    logger.info('track_id error:', err);
                    reject(err);
                }
                processThreadTracesRowData(threadId, rows, rowDatas);
                resolve(rowDatas);
            });
        });
    }

    async querySingleSliceByDepthTrackIdTime(depth: number, trackId: number, startTime: number): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT
                                    *
                                 FROM
                                    ${this.sliceTable}
                                 WHERE
                                    DEPTH = ?
                                    AND TRACK_ID = ?
                                    AND TIMESTAMP = ?`;
            this.db.all(sql, [ depth, trackId, startTime ], async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryDurationFromSliceByTimeRange(depth: number, endTime: number, startTime: number, trackId: number): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT DURATION FROM ${this.sliceTable} WHERE DEPTH = ?
                                    AND TIMESTAMP + DURATION <= ?
                                    AND TIMESTAMP >= ?
                                    AND TRACK_ID = ?`;
            this.db.all(sql, [ depth, endTime, startTime, trackId ], async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryFlowListByFlowId(flowId: string): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT * FROM ${this.flowTable} WHERE FLOW_ID = ?`;
            this.db.all(sql, [flowId], async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    };

    async queryLocationFlowByFlowId(flowId: string): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT FL.NAME, FL.CAT, FL.FLOW_ID as flowId,
                                    TH.PID, TH.TID, SL.DEPTH, SL.TIMESTAMP, FL.TYPE FROM ${this.threadTable} TH
                                 LEFT JOIN ${this.sliceTable} SL ON SL.TRACK_ID = TH.TRACK_ID
                                 LEFT JOIN ${this.flowTable} FL ON FL.TRACK_ID = SL.TRACK_ID
                                 WHERE FL.TIMESTAMP = SL.TIMESTAMP AND FL.flow_id = ?`;
            this.db.all(sql, [flowId], async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    };

    async querySimpleFlowListByTrackIdTime(trackId: number, startTime: number): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT name, flow_id as flowId, type FROM ${this.flowTable} WHERE
                                    TIMESTAMP = ? AND TRACK_ID = ?`;
            this.db.all(sql, [ startTime, trackId ], async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    };

    async querySliceFlowList(flowId: string, type: string): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT sl.timestamp, th.pid, th.tid, sl.depth FROM ${this.sliceTable} sl
                                 LEFT JOIN ${this.threadTable} th ON sl.TRACK_ID = th.TRACK_ID
                                 WHERE sl.TRACK_ID IN
                                    (
                                        SELECT TRACK_ID FROM ${this.flowTable} WHERE FLOW_ID = ?
                                            AND TYPE <> ?
                                    )
                                    AND TIMESTAMP IN
                                        (
                                            SELECT TIMESTAMP FROM ${this.flowTable} WHERE FLOW_ID = ?
                                            AND TYPE <> ?
                                        )`;
            this.db.all(sql, [ flowId, type, flowId, type ], async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    };

    async queryNotOverlapTime(table: Table, notOverlapTrackId: number, timeStamp: number, duration: number): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT duration FROM ${this.sliceTable}
                                WHERE track_id = ?
                              and timestamp >= ?
                              and timeStamp + duration <= ?`;
            this.db.all(sql, [ notOverlapTrackId, timeStamp, duration + timeStamp ], async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryComputeStatisticsData(stepCondition: string, param: string[]): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT
                                 sum(duration) as duration,
                                 accelerator_core as acceleratorCore
                                 FROM ${KERNEL_DETAIL_TABLE}
                                 WHERE accelerator_core in ('AI_CPU','AI_CORE') ${stepCondition}
                                 GROUP BY accelerator_core`;
            this.db.all(sql, param, async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error('queryComputeStatisticsData error:', err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryCommunicationStatisticsData(timestampCondition: string, param: number[]): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `select duration, t.thread_name as overlapType from (select sum(duration) as duration, track_id
                                    from ${this.sliceTable}
                                    where track_id in (select track_id from thread where thread_name in('Communication(Not Overlapped)', 'Communication'))
                                    ${timestampCondition}
                                    group by track_id) s
                                    left join thread t on s.track_id=t.track_id`;
            this.db.all(sql, param, async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error('queryCommunicationStatisticsData error:', err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryStepDuration(stepIdProfilerName: string): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `select duration,
                                    timestamp
                                    from ${this.sliceTable}
                                    where name=?`;
            this.db.get(sql, [stepIdProfilerName], async (err, result) => {
                if (err !== undefined && err !== null) {
                    logger.error('queryCommunicationStatisticsData error:', err.message);
                    reject(err);
                } else {
                    resolve(result);
                }
            });
        });
    }
}

/**
 * process threadTraces rowData
 *
 * @param threadId threadId
 * @param rows rows
 * @param rowDatas processed data
 */
function processThreadTracesRowData(threadId: number, rows: any, rowDatas: ThreadTrace[][]): void {
    rows.forEach((it: RowThreadTrace) => {
        if (rowDatas[it.depth] === undefined) {
            rowDatas[it.depth] = [];
        }
        const threadTrace = {
            name: it.name,
            duration: it.duration,
            startTime: it.start_time,
            endTime: it.start_time + it.duration,
            depth: it.depth,
            threadId,
        };
        rowDatas[it.depth].push(threadTrace);
    });
    for (let i = 0; i < rowDatas.length; i++) {
        if (rowDatas[i] === undefined) {
            rowDatas[i] = [];
        }
    }
}
