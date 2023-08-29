/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import * as sqlite from 'sqlite3';
import { getLoggerByName } from '../logger/loggger_configure';
import {
    CLUSTER_BASE_INFO_TABLE,
    COMMUNICATION_BAND_WIDTH_TABLE, COMMUNICATION_MATRIX,
    COMMUNICATION_TIME_INFO_TABLE,
    CREATE_CLUSTER_TABLE_SQL,
    CREATE_COMMUNICATION_BANDWIDTH_INFO_SQL,
    CREATE_COMMUNICATION_MATRIX_TABLE,
    CREATE_COMMUNICATION_TIME_INFO_SQL,
    CREATE_STEP_STATISTIC_INFO_TABLE_SQL,
    STEP_STATISTIC_INFO_TABLE,
} from '../common/sql_constant';
import {
    CommunicationBandWidthEntity,
    CommunicationMatrixInfoEntity,
    CommunicationTimeInfoEntity,
    StepStatisticEntity,
} from '../query/entity';
import { OperatorDetailsRequest } from '../query/communicationAnalysisData';

const logger = getLoggerByName('ClusterDatabase', 'info');

export class ClusterDatabase {
    private readonly clusterDb: sqlite.Database;
    private readonly _dbPath: string;
    private communicationTimeInfoCaches: any[] = [];
    private communicationBandWidthCaches: any[] = [];
    private communicationMatrixCaches: any[] = [];
    private readonly maxCachesSize = 100;
    count = 0;
    private timeInfoStat: sqlite.Statement | undefined;
    private bandWidthStat: sqlite.Statement | undefined;
    private matrixStat: sqlite.Statement | undefined;

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
                    .run(`DROP TABLE IF EXISTS ${COMMUNICATION_MATRIX}`)
                    .run(CREATE_COMMUNICATION_MATRIX_TABLE)
                    .run('PRAGMA synchronous = OFF')
                    .run('PRAGMA journal_mode = MEMORY', (err) => {
                        if (err) {
                            console.error(err.message);
                        }
                        console.log('Create cluster table end.');
                        resolve();
                    });
            });
        });
    }

    initStat(): void {
        if (this.timeInfoStat === undefined) {
            const valueParams = '(?,?,?,?,?,?,?,?,?,?,?,?)';
            const placeholders: string = (valueParams + ',').repeat(this.maxCachesSize - 1).concat(valueParams);
            const sql: string = `INSERT INTO ${COMMUNICATION_TIME_INFO_TABLE}
                                 (iteration_id, stage_id, rank_id, op_name, op_suffix, elapse_time, synchronization_time_ratio,
                                  synchronization_time, transit_time, wait_time_ratio, wait_time, idle_time)
                                 VALUES ${placeholders}`;
            this.timeInfoStat = this.clusterDb.prepare(sql);
        }
        if (this.bandWidthStat === undefined) {
            const valueParams = '(?,?,?,?,?,?,?,?,?,?,?,?)';
            const placeholders: string = (valueParams + ',').repeat(this.maxCachesSize - 1).concat((valueParams));
            const sql: string = `INSERT INTO ${COMMUNICATION_BAND_WIDTH_TABLE}
                                 (iteration_id, stage_id, rank_id, op_name, op_suffix, transport_type, bandwidth_size, bandwidth_utilization,
                                  large_package_ratio, size_distribution, transit_size, transit_time)
                                 VALUES ${placeholders}`;
            this.bandWidthStat = this.clusterDb.prepare(sql);
        }
        if (this.matrixStat === undefined) {
            const valueParams = '(?,?,?,?,?,?,?,?,?,?)';
            const placeholders: string = (valueParams + ',').repeat(this.maxCachesSize - 1).concat((valueParams));
            const sql: string = `INSERT INTO ${COMMUNICATION_MATRIX}
                                 (group_id, step, op_name, group_name, src_rank, dst_rank, transport_type,
                                  transit_size, transit_time, bandwidth)
                                 VALUES ${placeholders}`;
            this.matrixStat = this.clusterDb.prepare(sql);
        }
    }

    insertCommunicationTimeInfoList(dataList: CommunicationTimeInfoEntity[]): void {
        const paramsList: any[] = [];
        dataList.forEach((data) => {
            paramsList.push(data.iterationId, data.stageId, data.rankId, data.opName, data.opSuffix, data.elapseTime, data.synchronizationTimeRatio,
                data.synchronizationTime, data.transitTime, data.waitTimeRatio, data.waitTime, data.idleTime);
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
            const placeholders: string = dataList.map(() => '(?,?,?,?,?,?,?,?,?,?)').join(',');
            const sql: string = `INSERT INTO ${COMMUNICATION_TIME_INFO_TABLE}
                                 (iteration_id,stage_id, rank_id, op_name,op_suffix, elapse_time, synchronization_time_ratio,
                                  synchronization_time, transit_time, wait_time_ratio, wait_time, idle_time)
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

    async insertCommunicationMatrixInfo(data: CommunicationMatrixInfoEntity): Promise<void> {
        return new Promise((resolve, reject) => {
            this.communicationMatrixCaches.push(data);
            if (this.communicationMatrixCaches.length === this.maxCachesSize) {
                this.insertCommunicationMatrix(this.communicationMatrixCaches).finally(() => {
                    this.communicationMatrixCaches = [];
                    resolve();
                });
            } else {
                resolve();
            }
        });
    }

    async insertCommunicationMatrix(dataList: CommunicationMatrixInfoEntity[]): Promise<void> {
        return new Promise((resolve, reject) => {
            const paramsList: any[] = [];
            dataList.forEach((data) => {
                paramsList.push(data.groupId, data.step, data.opName, data.groupName,
                    data.srcRank, data.dstRank, data.transportType, data.transitSize, data.transitTime, data.bandwidth);
            });
            if (dataList.length === this.maxCachesSize) {
                if (this.matrixStat === undefined) {
                    this.initStat();
                }
                this.matrixStat?.reset().run(paramsList, (err) => {
                    if (err) {
                        logger.error(err.message);
                        reject(err);
                    }
                    resolve();
                });
            } else {
                const placeholders: string = dataList.map(() => '(?,?,?,?,?,?,?,?,?,?)').join(',');
                const sql: string = `INSERT INTO ${COMMUNICATION_MATRIX}
                                 (group_id, step, op_name, group_name, src_rank, dst_rank, transport_type,
                                  transit_size, transit_time, bandwidth)
                                 VALUES ${placeholders}`;
                this.clusterDb.run(sql, paramsList, (err) => {
                    if (err !== null) {
                        logger.error(err.message);
                        reject(err);
                    }
                    resolve();
                });
            }
        });
    }

    insertCommunicationBandWidthList(dataList: CommunicationBandWidthEntity[]): void {
        const paramsList: any[] = [];
        dataList.forEach((data) => {
            paramsList.push(data.iterationId, data.stageId, data.rankId, data.opName, data.opSuffix, data.transportType, data.bandwidthSize,
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
            const placeholders: string = dataList.map(() => '(?,?,?,?,?,?,?,?,?,?,?,?)').join(',');
            const sql: string = `INSERT INTO ${COMMUNICATION_BAND_WIDTH_TABLE}
                                 (iteration_id, stage_id, rank_id, op_name, op_suffix, transport_type, bandwidth_size,
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

    insertStepStatisticsInfo(data: StepStatisticEntity): void {
        const sql: string = `INSERT INTO ${STEP_STATISTIC_INFO_TABLE}
                             (rank_id, step_id, stage_id, compute_time,
                              pure_communication_time, overlap_communication_time,
                              communication_time, free_time, stage_time, bubble_time,
                              pure_communication_exclude_receive_time)
                             VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`;
        this.clusterDb.run(sql, [ data.rankId, data.stepId, data.stageId, data.computingTime,
            data.overlapCommunicationTime, data.communicationTime, data.freeTime, data.stageTime,
            data.bubbleTime, data.pureCommunicationExcludeReceiveTime ], (err) => {
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

    updateClusterBaseInfoRankList(data: any[]): void {
        const sql: string = `UPDATE ${CLUSTER_BASE_INFO_TABLE}
                             SET ranks=?, steps=?`;
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
        if (this.communicationMatrixCaches.length > 0) {
            this.insertCommunicationMatrix(this.communicationMatrixCaches);
            this.communicationMatrixCaches = [];
        }
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

    async querySummaryData(orderBy: string, stepIdList: string[], rankIdList: string[]): Promise<any> {
        return new Promise((resolve, reject) => {
            let param: string[] = [];
            let stepCondition = '';
            let rankCondition = '';
            if (stepIdList !== undefined && stepIdList.length > 0) {
                param = param.concat(stepIdList);
                stepCondition = 'and step_id in ('
                    .concat('?').concat((',?').repeat(stepIdList.length - 1)).concat(') ');
            }
            if (rankIdList !== undefined && rankIdList.length > 0) {
                param = param.concat(rankIdList);
                rankCondition = 'and rank_id in ('
                    .concat('?').concat((',?').repeat(rankIdList.length - 1)).concat(') ');
            }
            const sql: string = `SELECT
                                     rank_id as rankId,
                                     sum(ROUND(compute_time,2)) as computingTime,
                                     sum(ROUND(pure_communication_time,2)) as communicationNotOverLappedTime,
                                     sum(ROUND(overlap_communication_time,2)) as communicationOverLappedTime,
                                     sum(ROUND(free_time,2)) as freeTime
                                 FROM ${STEP_STATISTIC_INFO_TABLE}
                                 WHERE rank_id !='' ${stepCondition}  ${rankCondition}
                                 group by rank_id order by ${orderBy} desc`;
            this.clusterDb.all(sql, param, async (err, rows) => {
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
                                       collect_start_time as collectStartTime,
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

    async queryRankIds(iterationId: string): Promise<any> {
        const sql: string = `SELECT DISTINCT rank_id FROM ${COMMUNICATION_TIME_INFO_TABLE}
                             WHERE iteration_id = ? ORDER BY rank_id`;
        return this.executeSql(sql, [iterationId]);
    }

    async selectOperators(iterationId: string, rankIdList: string[]): Promise<any> {
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

    async queryDurationList(iterationId: string, rankIdList: string[], operatorName: string): Promise<any> {
        let sql: string = '';
        if (rankIdList.length === 0) {
            sql = `SELECT rank_id, ROUND(elapse_time, 4) as elapse_time,
                          ROUND(transit_time, 4) as transit_time,
                          ROUND(synchronization_time, 4) as synchronization_time,
                          ROUND(wait_time, 4) as wait_time,
                          ROUND(idle_time, 4) as idle_time,
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
                          ROUND(idle_time, 4) as idle_time,
                          ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio,
                          ROUND(wait_time_ratio, 4) as wait_time_ratio
                   FROM ${COMMUNICATION_TIME_INFO_TABLE}
                   WHERE iteration_id = ?
                     AND rank_id IN (${rankIdList.join(',')})
                     AND op_name = ?`;
        }
        return this.executeSql(sql, [ iterationId, operatorName ]);
    };

    async queryAllOperators(params: OperatorDetailsRequest): Promise<any> {
        const sql: string = `SELECT op_name,
                                    ROUND(elapse_time, 4) as elapse_time,
                                    ROUND(transit_time, 4) as transit_time,
                                    ROUND(synchronization_time, 4) as synchronization_time,
                                    ROUND(wait_time, 4) as wait_time,
                                    ROUND(idle_time, 4) as idle_time,
                                    ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio,
                                    ROUND(wait_time_ratio, 4) as wait_time_ratio
                             FROM (SELECT * FROM ${COMMUNICATION_TIME_INFO_TABLE}  ${params.orderBy ? 'ORDER BY ' + params.orderBy + ' ' + params.order : ''})
                             WHERE iteration_id = ?
                               AND rank_id = ?
                               AND op_name != 'Total Op Info'
                               LIMIT ?, ?`;
        return this.executeSql(sql, [ params.iterationId,
            params.rankId, (params.currentPage - 1) * params.pageSize, params.pageSize ]);
    }

    async queryOperatorsCount(iterationId: string, rankId: string): Promise<any> {
        const sql: string = `SELECT count(*) AS nums FROM ${COMMUNICATION_TIME_INFO_TABLE}
                             WHERE iteration_id = ?
                               AND rank_id = ?`;
        return this.executeSql(sql, [ iterationId, rankId ]);
    }

    async queryTotalOpInfoCount(iterationId: string, rankId: string): Promise<any> {
        const sql: string = `SELECT count(*) AS nums FROM ${COMMUNICATION_TIME_INFO_TABLE}
                             WHERE iteration_id = ?
                               AND rank_id = ?
                               AND op_name = 'Total Op Info'`;
        return this.executeSql(sql, [ iterationId, rankId ]);
    }

    async queryBandwidthData(iterationId: string, rankId: string, operatorName: string): Promise<any> {
        const sql: string = `SELECT transport_type,
                                    ROUND(transit_size, 4) as transit_size,
                                    ROUND(transit_time, 4) as transit_time,
                                    ROUND(bandwidth_size, 4) as bandwidth_size,
                                    ROUND(large_package_ratio, 4)  as large_package_ratio
                             FROM ${COMMUNICATION_BAND_WIDTH_TABLE}
                             WHERE iteration_id = ?
                               AND rank_id = ?
                               AND op_name = ?`;
        return this.executeSql(sql, [ iterationId, rankId, operatorName ]);
    }

    async queryDistributionData(iterationId: string, rankId: string, operatorName: string, transportType: string): Promise<any> {
        const sql: string = `SELECT size_distribution FROM ${COMMUNICATION_BAND_WIDTH_TABLE}
                             WHERE iteration_id = ?
                               AND rank_id = ?
                               AND op_name = ?
                               AND transport_type = ?`;
        return this.executeSql(sql, [ iterationId, rankId, operatorName, transportType ]);
    }

    async queryMatrixList(step: string, operatorName: string, groupId: string): Promise<any> {
        let sql: string = '';
        if (groupId === null || groupId === '') {
            sql = `SELECT src_rank as srcRank, dst_rank as dstRank,
                          transport_type as transportType,
                          ROUND(transit_size, 4) as transitSize,
                          ROUND(transit_time, 4) as transitTime,
                          ROUND(bandwidth, 4) as bandwidth
                   FROM ${COMMUNICATION_MATRIX}
                   WHERE step = ?
                     AND op_name = ?`;
            return this.executeSql(sql, [ step, operatorName ]);
        } else {
            sql = `SELECT src_rank as srcRank, dst_rank as dstRank,
                          transport_type as transportType,
                          ROUND(transit_size, 4) as transitSize,
                          ROUND(transit_time, 4) as transitTime,
                          ROUND(bandwidth, 4) as bandwidth
                   FROM ${COMMUNICATION_MATRIX}
                   WHERE group_id = ? AND
                      step = ?
                     AND op_name = ?`;
            return this.executeSql(sql, [ groupId, step, operatorName ]);
        }
    }

    async getGroups(): Promise<any> {
        return new Promise((resolve, reject) => {
            this.clusterDb.all(`SELECT DISTINCT group_id as groupId FROM ${COMMUNICATION_MATRIX}`, [], (err, rows) => {
                if (err) {
                    logger.error('get group error:', err);
                    reject(err);
                }
                resolve(rows);
            });
        });
    }
}
