/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { getLoggerByName } from '../logger/loggger_configure';
import { CLUSTER_DATABASE } from '../database/tableManager';
import fs from 'fs';
import { mapperToBandWidthEntity, mapperToStepStatisticsInfo, mapperToTimeInfoEntity } from '../utils/mapper_util';
import JSONStream from 'JSONStream';
import readline from 'readline';
import { getFolderSize } from '../utils/common_util';
import { Client } from '../types';
import { toInteger, toNumber } from 'lodash';

const logger = getLoggerByName('communicationParser', 'info');

export function parseCommunicationFile(filePathArr: string[], client?: Client): void {
    const start = new Date().getTime();
    for (const filePath of filePathArr) {
        logger.info('start save communication data into db ,file:', filePath);
        const stream = fs.createReadStream(filePath, { encoding: 'utf-8' });
        const parser = JSONStream.parse([ /.*/, { recurse: true }, /^[0-9]{1,5}/, { emitPath: true } ]);
        stream.pipe(parser);
        let countTimeInfo = 0; let countBandWidth = 0;
        parser.on('data', (data: any) => {
            const tempPath = data.path;
            const tempData = data.value;
            const tempStepId = tempPath[1].replace('step', '');
            const tempOpName = tempPath[2];
            const stageId = tempPath[0];
            const tempRankId = tempPath[3];
            if (tempPath[4] === 'Communication Time Info') {
                if (typeof tempData !== 'string') {
                    const tempTimeInfo = mapperToTimeInfoEntity(tempRankId, tempOpName, tempStepId, tempData, stageId);
                    CLUSTER_DATABASE.insertCommunicationTimeInfo(tempTimeInfo);
                }
                countTimeInfo++;
            } else if (tempPath[4] === 'Communication Bandwidth Info') {
                const keys = Object.keys(tempData);
                keys.forEach(key => {
                    const obj = data.value[key];
                    const bandWidth = mapperToBandWidthEntity(tempRankId, tempOpName, tempStepId, key, obj, stageId);
                    CLUSTER_DATABASE.insertCommunicationBandWidth(bandWidth);
                    countBandWidth++;
                });
            }
        });
        parser.on('error', (err: any) => {
            client?.notify('parseCommunication/success', { parseResult: 'fail', parseName: 'communication' });
            logger.error('parseCommunication occur error: ', err);
        });
        parser.on('end', () => {
            CLUSTER_DATABASE.saveCommunicationData();
            client?.notify('parseCommunication/success', { parseResult: 'ok', parseName: 'communication' });
            const end = new Date().getTime();
            logger.info('cost time :', end - start);
            logger.info('Finished parsing file. time info:{}, bandWidth:{}', countTimeInfo, countBandWidth);
        });
    }
}

export function parseStepStatisticsFile(filePathArr: string[]): void {
    let count = 0;
    const stepList: string[] = [];
    const rankList: string[] = [];
    const start = new Date().getTime();
    filePathArr.forEach(filePath => {
        logger.info('import step statistics file start:', filePath);
        const stream = fs.createReadStream(filePath);
        const rl = readline.createInterface({
            input: stream,
            crlfDelay: 0,
        });
        rl.on('line', (line) => {
            if (!line.trim().startsWith('Step')) {
                const arr = line.split(',');
                if (arr[1] === 'rank') {
                    if (!rankList.includes(arr[2])) {
                        rankList.push(arr[2]);
                    }
                    const tempStep = toInteger(arr[0]).toString();
                    if (!stepList.includes(tempStep)) {
                        stepList.push(tempStep);
                    }
                    CLUSTER_DATABASE.insertStepStatisticsInfo(mapperToStepStatisticsInfo(arr));
                    count++;
                }
            }
        });
        rl.on('close', () => {
            // 更新rankList 和 stepList
            CLUSTER_DATABASE.updateClusterBaseInfoRankList(
                [ JSON.stringify(rankList.sort((a, b) => {
                    return toNumber(a) - toNumber(b);
                })), JSON.stringify(stepList.sort((a, b) => {
                    return toNumber(a) - toNumber(b);
                })) ]);
            const end = new Date().getTime();
            // 读取完成
            logger.info('import step statistics file end, total line:', count, 'cost time:', end - start);
        });
    });
}

export function saveClusterBaseInfo(selectedPath: string): void {
    if (selectedPath == null) return;
    const data = [];
    data.push(selectedPath);
    data.push(JSON.stringify([]));
    data.push(JSON.stringify([]));
    data.push(new Date());
    data.push(1000);
    data.push(getFolderSize(selectedPath));
    logger.info('start save cluster base info', data);
    CLUSTER_DATABASE.insertClusterBaseInfo(data);
}
