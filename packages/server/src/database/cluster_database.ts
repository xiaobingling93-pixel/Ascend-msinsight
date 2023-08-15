/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import * as sqlite from 'sqlite3';
import { getLoggerByName } from '../logger/loggger_configure';
import {
    CLUSTER_BASE_INFO_TABLE,
    COMMUNICATION_BAND_WIDTH_TABLE,
    COMMUNICATION_TIME_INFO_TABLE,
    CREATE_CLUSTER_TABLE_SQL,
    CREATE_COMMUNICATION_BANDWIDTH_INFO_SQL,
    CREATE_COMMUNICATION_TIME_INFO_SQL,
    CREATE_STEP_STATISTIC_INFO_TABLE_SQL,
    STEP_STATISTIC_INFO_TABLE,
} from '../common/sql_constant';
import { CommunicationBandWidthEntity, CommunicationTimeInfoEntity } from '../query/entity';

const logger = getLoggerByName('ClusterDatabase', 'info');

export class ClusterDatabase {
    private readonly clusterDb: sqlite.Database;
    private readonly _dbPath: string;
    private communicationTimeInfoCaches: any[] = [];
    private communicationBandWidthCaches: any[] = [];
    private readonly maxCachesSize = 100;
    count = 0;
    private timeInfoStat: sqlite.Statement | undefined;
    private bandWidthStat: sqlite.Statement | undefined;

    constructor(dbPath: string) {
        this.clusterDb = new sqlite.Database(dbPath, sqlite.OPEN_READWRITE | sqlite.OPEN_CREATE | sqlite.OPEN_SHAREDCACHE, (err) => {
            if (err !== null) {
                logger.error(err.message);
            }
            logger.info('Connect to database.');
        });
        this._dbPath = dbPath;
    }

    async createClusterTable(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.clusterDb.serialize(() => {
                this.clusterDb
                    .run(`DROP TABLE IF EXISTS ${COMMUNICATION_TIME_INFO_TABLE}`)
                    .run(CREATE_COMMUNICATION_TIME_INFO_SQL)
                    .run(`DROP TABLE IF EXISTS ${COMMUNICATION_BAND_WIDTH_TABLE}`)
                    .run(CREATE_COMMUNICATION_BANDWIDTH_INFO_SQL)
                    .run(`DROP TABLE IF EXISTS ${CLUSTER_BASE_INFO_TABLE}`)
                    .run(CREATE_CLUSTER_TABLE_SQL)
                    .run(`DROP TABLE IF EXISTS ${STEP_STATISTIC_INFO_TABLE}`)
                    .run(CREATE_STEP_STATISTIC_INFO_TABLE_SQL)
                    .run('PRAGMA synchronous = OFF')
                    .run('PRAGMA journal_mode = MEMORY', (err) => {
                        if (err) {
                            logger.error(err.message);
                        }
                        console.log('Create cluster table end.');
                        resolve();
                    });
            });
        });
    }

    initStat(): void {
        if (this.timeInfoStat === undefined) {
            const valueParams = '(?,?,?,?,?,?,?,?,?)';
            const placeholders: string = (valueParams + ',').repeat(this.maxCachesSize - 1).concat(valueParams);
            const sql: string = `INSERT INTO ${COMMUNICATION_TIME_INFO_TABLE}
                                 (iteration_id, rank_id, op_name, elapse_time, synchronization_time_ratio,
                                  synchronization_time, transit_time, wait_time_ratio, wait_time)
                                 VALUES ${placeholders}`;
            this.timeInfoStat = this.clusterDb.prepare(sql);
        }
        if (this.bandWidthStat === undefined) {
            const valueParams = '(?,?,?,?,?,?,?,?,?,?)';
            const placeholders: string = (valueParams + ',').repeat(this.maxCachesSize - 1).concat((valueParams));
            const sql: string = `INSERT INTO ${COMMUNICATION_BAND_WIDTH_TABLE}
                                 (iteration_id, rank_id, op_name, transport_type, bandwidth_size, bandwidth_utilization,
                                  large_package_ratio, size_distribution, transit_size, transit_time)
                                 VALUES ${placeholders}`;
            this.bandWidthStat = this.clusterDb.prepare(sql);
        }
    }

    insertCommunicationTimeInfoList(dataList: CommunicationTimeInfoEntity[]): void {
        const paramsList: any[] = [];
        dataList.forEach((data) => {
            paramsList.push(data.iterationId, data.rankId, data.opName, data.elapseTime, data.synchronizationTimeRatio,
                data.synchronizationTime, data.transitTime, data.waitTimeRatio, data.waitTime);
        });
        if (dataList.length === this.maxCachesSize) {
            if (this.timeInfoStat === undefined) {
                this.initStat();
            }
            this.timeInfoStat?.reset().run(paramsList, (err) => {
                if (err) {
                    logger.error(err.message);
                }
            });
        } else {
            const placeholders: string = dataList.map(() => '(?,?,?,?,?,?,?,?,?)').join(',');
            const sql: string = `INSERT INTO ${COMMUNICATION_TIME_INFO_TABLE}
                                 (iteration_id, rank_id, op_name, elapse_time, synchronization_time_ratio,
                                  synchronization_time, transit_time, wait_time_ratio, wait_time)
                                 VALUES ${placeholders}`;
            this.clusterDb.run(sql, paramsList, (err) => {
                if (err !== null) {
                    logger.error(err.message);
                }
            });
        }
    }

    insertCommunicationTimeInfo(data: CommunicationTimeInfoEntity): void {
        this.communicationTimeInfoCaches.push(data);
        if (this.communicationTimeInfoCaches.length >= this.maxCachesSize) {
            this.insertCommunicationTimeInfoList(this.communicationTimeInfoCaches);
            this.communicationTimeInfoCaches = [];
        }
    }

    insertCommunicationBandWidthList(dataList: CommunicationBandWidthEntity[]): void {
        const paramsList: any[] = [];
        dataList.forEach((data) => {
            paramsList.push(data.iterationId, data.rankId, data.opName, data.transportType, data.bandwidthSize,
                data.bandwidthUtilization, data.largePackageRatio, data.sizeDistribution, data.transitSize, data.transitTime);
        });
        if (dataList.length === this.maxCachesSize) {
            if (this.bandWidthStat === undefined) {
                this.initStat();
            }
            this.bandWidthStat?.reset().run(paramsList, (err) => {
                if (err) {
                    logger.error(err.message);
                }
            });
        } else {
            const placeholders: string = dataList.map(() => '(?,?,?,?,?,?,?,?,?,?)').join(',');
            const sql: string = `INSERT INTO ${COMMUNICATION_BAND_WIDTH_TABLE}
                                 (iteration_id, rank_id, op_name, transport_type, bandwidth_size,
                                  bandwidth_utilization,
                                  large_package_ratio, size_distribution, transit_size, transit_time)
                                 VALUES ${placeholders}`;
            this.clusterDb.run(sql, paramsList, (err) => {
                if (err !== null) {
                    logger.error(err.message);
                }
            });
        }
    }

    insertCommunicationBandWidth(data: CommunicationBandWidthEntity): void {
        this.communicationBandWidthCaches.push(data);
        if (this.communicationBandWidthCaches.length >= this.maxCachesSize) {
            this.insertCommunicationBandWidthList(this.communicationBandWidthCaches);
            this.communicationBandWidthCaches = [];
        }
    }

    insertStepStatisticsInfo(data: any[]): void {
        const sql: string = `INSERT INTO ${STEP_STATISTIC_INFO_TABLE}
                             (rank_id, step_id, stage_id, computing_time,
                              pure_communication_time, overlap_communication_time,
                              communication_time, free_time, bubble_time,
                              pure_communication_exclude_receive_time)
                             VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`;
        this.clusterDb.run(sql, data, (err) => {
            if (err !== null) {
                logger.error(err.message);
            }
        });
    }

    insertClusterBaseInfo(data: any[]): void {
        const sql: string = `INSERT INTO ${CLUSTER_BASE_INFO_TABLE}
                             (file_path, ranks, steps, collect_start_time,
                              collect_duration, data_size)
                             VALUES (?, ?, ?, ?, ?, ?)`;
        this.clusterDb.run(sql, data, (err) => {
            if (err !== null) {
                logger.error(err.message);
            }
        });
    }

    saveCommunicationData(): void {
        if (this.communicationTimeInfoCaches.length > 0) {
            this.insertCommunicationTimeInfoList(this.communicationTimeInfoCaches);
            this.communicationTimeInfoCaches = [];
        }
        if (this.communicationBandWidthCaches.length > 0) {
            this.insertCommunicationBandWidthList(this.communicationBandWidthCaches);
            this.communicationBandWidthCaches = [];
        }
        this.close();
    }

    close(): void {
        this.timeInfoStat?.finalize(() => {
            logger.info('timeInfoStat finalize.');
        });
        this.bandWidthStat?.finalize(() => {
            logger.info('bandWidthStat finalize.');
        });
        this.clusterDb.close((err) => {
            if (err !== null) {
                logger.error(err.message);
            }
            logger.info('Close the database connection.');
        });
    }

    async querySummaryData(orderBy: string): Promise<any> {
        return new Promise((resolve, reject) => {
            const sql: string = `SELECT
                                         rank_id as rankId,
                                         sum(compute_time) as computingTime,
                                         sum(pure_communication_time) as communicationNotOverLappedTime,
                                         sum(overlap_communication_time) as communicationOverLappedTime,
                                         sum(free_time) as freeTime
                                     FROM ${STEP_STATISTIC_INFO_TABLE}
                                     group by rank_id order by ?`;
            this.clusterDb.all(sql, [orderBy], async (err, rows) => {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async getTotal(tableName: string): Promise<any> {
        return new Promise((resolve, reject) => {
            this.clusterDb.all(`select count(1) as num from ${tableName}`, [], (err, rows) => {
                if (err) {
                    logger.error('getTotal error:', err);
                    reject(err);
                }
                resolve(rows[0]);
            });
        });
    }

    async getRankIdList(): Promise<any> {
        return new Promise((resolve, reject) => {
            this.clusterDb.all(`select distinct rank_id as rankId from ${STEP_STATISTIC_INFO_TABLE}`, [], (err, rows) => {
                if (err) {
                    logger.error('getRankIdList error:', err);
                    reject(err);
                }
                resolve(rows);
            });
        });
    }

    async getStepIdList(): Promise<any> {
        return new Promise((resolve, reject) => {
            this.clusterDb.all(`select distinct step_id as stepId from ${STEP_STATISTIC_INFO_TABLE}`, [], (err, rows) => {
                if (err) {
                    logger.error('getStepIdList error:', err);
                    reject(err);
                }
                resolve(rows);
            });
        });
    }

    async queryBaseInfo(): Promise<any> {
        return new Promise((resolve, reject) => {
            this.clusterDb.all(`select file_path as filePath,
                                       ranks,
                                       steps,
                                       data_size as dataSize
                                from ${CLUSTER_BASE_INFO_TABLE}`,
            [], (err, rows) => {
                if (err) {
                    logger.error('queryBaseInfo error:', err);
                    reject(err);
                }
                resolve(rows);
            });
        });
    }

    async executeSql(sql: string, params: any): Promise<any> {
        return new Promise((resolve, reject) => {
            this.clusterDb.all(sql, params, function (err, rows) {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryIterationIds(): Promise<any> {
        const sql: string = `SELECT DISTINCT iteration_id FROM ${COMMUNICATION_TIME_INFO_TABLE} ORDER BY iteration_id`;
        return new Promise((resolve, reject) => {
            this.clusterDb.all(sql, function (err, rows) {
                if (err !== undefined && err !== null) {
                    logger.error(err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }

    async queryRankIds(iterationId: number): Promise<any> {
        const sql: string = `SELECT DISTINCT rank_id FROM ${COMMUNICATION_TIME_INFO_TABLE}
                                 WHERE iteration_id = ? ORDER BY rank_id`;
        return this.executeSql(sql, [iterationId]);
    }

    async selectOperators(iterationId: number, rankIdList: number[]): Promise<any> {
        let sql: string = '';
        if (rankIdList.length === 0) {
            sql = `SELECT DISTINCT op_name FROM (
                   SELECT op_name FROM ${COMMUNICATION_TIME_INFO_TABLE}
                                 WHERE iteration_id = ? ORDER BY op_name)`;
        } else {
            sql = `SELECT DISTINCT op_name FROM (
                   SELECT op_name FROM ${COMMUNICATION_TIME_INFO_TABLE}
                                 WHERE iteration_id = ?
                                 AND rank_id IN (${rankIdList.join(',')}) ORDER BY op_name)`;
        }
        return this.executeSql(sql, [iterationId]);
    }

    async queryDurationList(iterationId: number, rankIdList: number[], operatorName: string): Promise<any> {
        let sql: string = '';
        if (rankIdList.length === 0) {
            sql = `SELECT rank_id, ROUND(elapse_time, 4) as elapse_time,
                                  ROUND(transit_time, 4) as transit_time,
                                  ROUND(synchronization_time, 4) as synchronization_time,
                                  ROUND(wait_time, 4) as wait_time,
                                  ROUND(elapse_time - transit_time - wait_time, 4) as idle_time,
                                  ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio,
                                  ROUND(wait_time_ratio, 4) as wait_time_ratio
                                  FROM ${COMMUNICATION_TIME_INFO_TABLE}
                                  WHERE iteration_id = ?
                                  AND op_name = ?`;
        } else {
            sql = `SELECT rank_id, ROUND(elapse_time, 4) as elapse_time,
                                   ROUND(transit_time, 4) as transit_time,
                                   ROUND(synchronization_time, 4) as synchronization_time,
                                   ROUND(wait_time, 4) as wait_time,
                                   ROUND(elapse_time - transit_time - wait_time, 4) as idle_time,
                                   ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio,
                                   ROUND(wait_time_ratio, 4) as wait_time_ratio
                                   FROM ${COMMUNICATION_TIME_INFO_TABLE}
                                  WHERE iteration_id = ?
                                  AND rank_id IN (${rankIdList.join(',')})
                                  AND op_name = ?`;
        }
        return this.executeSql(sql, [ iterationId, operatorName ]);
    };

    async queryAllOperators(iterationId: number, rankId: number, pageSize: number, currentPage: number): Promise<any> {
        const sql: string = `SELECT op_name,
                                    ROUND(elapse_time, 4) as elapse_time,
                                    ROUND(transit_time, 4) as transit_time,
                                    ROUND(synchronization_time, 4) as synchronization_time,
                                    ROUND(wait_time, 4) as wait_time,
                                    ROUND(elapse_time - transit_time - wait_time, 4) as idle_time,
                                    ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio,
                                    ROUND(wait_time_ratio, 4) as wait_time_ratio
                                    FROM ${COMMUNICATION_TIME_INFO_TABLE}
                                    WHERE iteration_id = ?
                                    AND rank_id = ?
                                    LIMIT ?, ?`;
        return this.executeSql(sql, [ iterationId, rankId, (currentPage - 1) * pageSize, pageSize ]);
    }

    async queryOperatorsCount(iterationId: number, rankId: number): Promise<any> {
        const sql: string = `SELECT count(*) AS nums FROM ${COMMUNICATION_TIME_INFO_TABLE}
                             WHERE iteration_id = ?
                             AND rank_id = ?`;
        return this.executeSql(sql, [ iterationId, rankId ]);
    }

    async queryBandwidthData(iterationId: number, rankId: number, operatorName: string): Promise<any> {
        const sql: string = `SELECT transport_type,
                                    ROUND(transit_size, 4) as transit_size,
                                    ROUND(transit_time, 4) as transit_time,
                                    ROUND(bandwidth_size, 4) as bandwidth_size,
                                    ROUND(bandwidth_utilization, 4) as bandwidth_utilization,
                                    ROUND(large_package_ratio, 4)  as large_package_ratio
                                    FROM ${COMMUNICATION_BAND_WIDTH_TABLE}
                                    WHERE iteration_id = ?
                                    AND rank_id = ?
                                    AND op_name = ?`;
        return this.executeSql(sql, [ iterationId, rankId, operatorName ]);
    }

    async queryDistributionData(iterationId: number, rankId: number, operatorName: string, transportType: string): Promise<any> {
        const sql: string = `SELECT size_distribution FROM ${COMMUNICATION_BAND_WIDTH_TABLE}
                             WHERE iteration_id = ?
                             AND rank_id = ?
                             AND op_name = ?
                             AND transport_type = ?`;
        return this.executeSql(sql, [ iterationId, rankId, operatorName, transportType ]);
    }
}
