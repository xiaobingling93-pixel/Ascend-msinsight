import sqlite, { Statement } from 'sqlite3';

export class Table {
    private readonly db: sqlite.Database;
    private readonly sliceTable = 'slice';
    private readonly threadTable = 'thread';
    private readonly processTable = 'process';
    private readonly flowTable = 'flow';
    private readonly idIndex = 'id_index';
    private readonly trackIdTimeIndex = 'track_id_time_index';
    private dataCaches: any[] = [];
    private readonly maxCachesSize = 1000;
    count = 0;
    private readonly depthCache = new Map<number, number[]>();
    private stat: Statement | undefined;

    constructor(dbPath: string) {
        this.db = new sqlite.Database(dbPath, sqlite.OPEN_READWRITE | sqlite.OPEN_CREATE | sqlite.OPEN_SHAREDCACHE, (err) => {
            if (err !== null) {
                console.error(err.message);
            }
            console.log('Connect to database.');
        });
    }

    async createTable(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.db.serialize(() => {
                this.db.run(`DROP TABLE IF EXISTS ${this.sliceTable}`)
                    .run(`CREATE TABLE ${this.sliceTable} (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, duration INTEGER, name TEXT, depth INTEGER, track_id INTEGER, args TEXT)`)
                    .run(`DROP TABLE IF EXISTS ${this.threadTable}`)
                    .run(`CREATE TABLE ${this.threadTable} (track_id INTEGER PRIMARY KEY, tid INTEGER, pid TEXT, thread_name TEXT, thread_sort_index INTEGER)`)
                    .run(`DROP TABLE IF EXISTS ${this.processTable}`)
                    .run(`CREATE TABLE ${this.processTable} (pid TEXT PRIMARY KEY, process_name TEXT, label TEXT, process_sort_index INTEGER)`)
                    .run(`DROP TABLE IF EXISTS ${this.flowTable}`)
                    .run(`CREATE TABLE ${this.flowTable} (flow_id TEXT PRIMARY KEY, name TEXT, cat TEXT, start_track_id INTEGER, start_timestamp INTEGER, end_track_id INTEGER, end_timestamp INTEGER)`)
                    .run(`DROP INDEX IF EXISTS ${this.idIndex}`)
                    .run(`DROP INDEX IF EXISTS ${this.trackIdTimeIndex}`)
                    .run('PRAGMA synchronous = OFF')
                    .run('PRAGMA journal_mode = MEMORY', (err) => {
                        if (err) {
                            console.error(err.message);
                        }
                        console.log('Create table end.');
                        resolve();
                    });
            });
        });
    }

    async close(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.stat?.finalize(() => {
                console.log('stat finalize.');
            });
            this.db.close((err) => {
                if (err !== null) {
                    console.error(err.message);
                    reject(err);
                }
                console.log('Close the database connection.');
                resolve();
            });
        });
    }

    // ts: double us
    async insertSliceList(dataList: Array<{ name: string; ts: number; dur: number; track_id: number; args: any }>): Promise<void> {
        return new Promise((resolve, reject) => {
            if (this.stat === undefined && dataList.length === this.maxCachesSize) {
                const placeholders: string = dataList.map(() => '(?,?,?,?,?)').join(',');
                const sql: string = `INSERT INTO ${this.sliceTable} (timestamp, duration, name, track_id, args) VALUES ${placeholders}`;
                this.stat = this.db.prepare(sql);
            }
            const paramsList: any[] = [];
            dataList.forEach((data) => { paramsList.push(data.ts, data.dur, data.name, data.track_id, JSON.stringify(data.args)); });
            if (dataList.length === this.maxCachesSize) {
                this.stat?.reset().run(paramsList, (err) => {
                    if (err) {
                        console.error(err.message);
                        reject(err);
                    }
                    ++this.count;
                    if (this.count % 100 === 0) {
                        console.log('insertSliceList count.', this.count * dataList.length);
                    }
                    resolve();
                });
            } else {
                const placeholders: string = dataList.map(() => '(?,?,?,?,?)').join(',');
                const sql: string = `INSERT INTO ${this.sliceTable} (timestamp, duration, name, track_id, args) VALUES ${placeholders}`;
                this.db.run(sql, paramsList, (err) => {
                    if (err !== null) {
                        console.error(err.message);
                        reject(err);
                    }
                    ++this.count;
                    if (this.count % 100 === 0) {
                        console.log('insertSliceList count.', this.count * dataList.length);
                    }
                    resolve();
                });
            }
        });
    }

    async insertSlice(data: { name: string; ts: number; dur: number; track_id: number; args: any }): Promise<void> {
        return new Promise((resolve, reject) => {
            this.dataCaches.push(data);
            if (this.dataCaches.length >= this.maxCachesSize) {
                this.insertSliceList(this.dataCaches).finally(() => {
                    resolve();
                    this.dataCaches = [];
                });
            } else {
                resolve();
            }
        });
    }

    updateProcessName(pid: string, name: string): void {
        const sql: string = `INSERT INTO ${this.processTable} (pid, process_name) VALUES (?, ?) ON CONFLICT (pid) DO UPDATE SET process_name = excluded.process_name`;
        this.db.run(sql, [ pid, name ], (err) => {
            if (err !== null) {
                console.error(err.message);
            }
        });
    }

    updateProcessLabel(pid: string, label: string): void {
        const sql: string = `INSERT INTO ${this.processTable} (pid, label) VALUES (?, ?) ON CONFLICT (pid) DO UPDATE SET label = excluded.label;`;
        this.db.run(sql, [ pid, label ], (err) => {
            if (err !== null) {
                console.error(err.message);
            }
        });
    }

    updateProcessSortIndex(pid: string, sortIndex: number): void {
        const sql: string = `INSERT INTO ${this.processTable} (pid, process_sort_index) VALUES (?, ?) ON CONFLICT (pid) DO UPDATE SET process_sort_index = excluded.process_sort_index;`;
        this.db.run(sql, [ pid, sortIndex ], (err) => {
            if (err !== null) {
                console.error(err.message);
            }
        });
    }

    updateThreadName(trackId: number, tid: number, pid: string, threadName: string): void {
        const sql: string = `INSERT INTO ${this.threadTable} (track_id, tid, pid, thread_name) VALUES (?, ?, ?, ?) ON CONFLICT (track_id) DO UPDATE SET tid = excluded.tid, pid = excluded.pid, thread_name = excluded.thread_name`;
        this.db.run(sql, [ trackId, tid, pid, threadName ], (err) => {
            if (err !== null) {
                console.error(err.message);
            }
        });
    }

    updateThreadSortIndex(trackId: number, sortIndex: number): void {
        const sql: string = `INSERT INTO ${this.threadTable} (track_id, thread_sort_index) VALUES (?, ?) ON CONFLICT (track_id) DO UPDATE SET thread_sort_index = excluded.thread_sort_index`;
        this.db.run(sql, [ trackId, sortIndex ], (err) => {
            if (err !== null) {
                console.error(err.message);
            }
        });
    }

    updateFlowStartPosition(data: { name: string; cat: string; id: string; track_id: number; ts: number }): void {
        const sql: string = `INSERT INTO ${this.flowTable} (flow_id, name, cat, start_track_id, start_timestamp) VALUES (?, ?, ?, ?, ?) ON CONFLICT (flow_id) DO UPDATE SET start_track_id = excluded.start_track_id, start_timestamp = excluded.start_timestamp`;
        this.db.run(sql, [ data.id, data.name, data.cat, data.track_id, data.ts ], (err) => {
            if (err !== null) {
                console.error(err.message);
            }
        });
    }

    updateFlowEndPosition(data: { name: string; cat: string; id: string; track_id: number; ts: number }): void {
        const sql: string = `INSERT INTO ${this.flowTable} (flow_id, name, cat, end_track_id, end_timestamp) VALUES (?, ?, ?, ?, ?) ON CONFLICT (flow_id) DO UPDATE SET end_track_id = excluded.end_track_id, end_timestamp = excluded.end_timestamp`;
        this.db.run(sql, [ data.id, data.name, data.cat, data.track_id, data.ts ], (err) => {
            if (err !== null) {
                console.error(err.message);
            }
        });
    }

    async beginTransaction(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.db.run('BEGIN', (err) => {
                if (err) {
                    console.error(err.message);
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
                    console.error(err.message);
                    reject(err);
                }
                resolve();
            });
        });
    }

    async commitData(): Promise<void> {
        if (this.dataCaches.length > 0) {
            await this.insertSliceList(this.dataCaches);
            this.dataCaches = [];
        }
    }

    async selectData(sql: string, params: any): Promise<any> {
        return new Promise((resolve, reject) => {
            this.db.all(sql, params, (err, rows) => {
                if (err !== undefined && err !== null) {
                    console.error(err.message);
                    reject(err);
                } else {
                    const result = rows.map(row => {
                        return row;
                    });
                    resolve(result);
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
                            console.error(err.message);
                            reject(err);
                        }
                        console.log('create id index end.', new Date().getTime() - start);
                        resolve();
                    });
            });
        });
    }
}
