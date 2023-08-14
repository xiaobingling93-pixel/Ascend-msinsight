/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { getLoggerByName } from '../logger/loggger_configure';
import { CLUSTER_DATABASE } from '../database/tableManager';
import fs from 'fs';
import { mapperToBandWidthEntity, mapperToTimeInfoEntity } from '../utils/mapper_util';
import JSONStream from 'JSONStream';
import readline from 'readline';

const logger = getLoggerByName('communicationParser', 'info');

export function parseCommunicationFile(filePathArr: string[]): void {
    const start = new Date().getTime();
    for (const filePath of filePathArr) {
        logger.info('start save communication data into db ,file:', filePath);
        const stream = fs.createReadStream(filePath, { encoding: 'utf-8' });
        const parser = JSONStream.parse([ /.*/, { recurse: true }, /[0-9]{1,5}/, { emitPath: true } ]);
        stream.pipe(parser);
        let countTimeInfo = 0; let countBandWidth = 0;
        parser.on('data', (data: any) => {
            const tempPath = data.path;
            const tempData = data.value;
            const tempOpName = tempPath[0];
            const tempRankId = tempPath[1];
            if (tempPath[2] === 'Communication Time Info') {
                if (typeof tempData !== 'string') {
                    const tempTimeInfo = mapperToTimeInfoEntity(tempRankId, tempOpName, tempData);
                    CLUSTER_DATABASE.insertCommunicationTimeInfo(tempTimeInfo);
                }
                countTimeInfo++;
            } else if (tempPath[2] === 'Communication Bandwidth Info') {
                const keys = Object.keys(tempData);
                keys.forEach(key => {
                    const obj = data.value[key];
                    const bandWidth = mapperToBandWidthEntity(tempRankId, tempOpName, key, obj);
                    CLUSTER_DATABASE.insertCommunicationBandWidth(bandWidth);
                    countBandWidth++;
                });
            }
        });
        parser.on('error', (err: any) => {
            logger.error(err);
        });
        parser.on('end', () => {
            CLUSTER_DATABASE.saveData();
            const end = new Date().getTime();
            logger.info('cost time :', end - start);
            logger.info('Finished parsing file. time info:{}, bandWidth:{}', countTimeInfo, countBandWidth);
        });
    }
}

export function parseStepStatisticsFile(filePathArr: string[], callback?: (rankId: string, err?: Error) => void): void {
    let count = 0;
    filePathArr.forEach(filePath => {
        const stream = fs.createReadStream(filePath);
        const rl = readline.createInterface({
            input: stream,
            crlfDelay: 0,
        });
        rl.on('line', (line) => {
            const arr = line.split(',');
            CLUSTER_DATABASE.insertStepStatisticsInfo(arr);
            count++;
        });
        rl.on('close', () => {
            // 读取完成
            logger.log('import step statistics file end, total line:{}', count);
        });
    });
}

export function saveClusterBaseInfo(selectedPath: string | null): void {
    if (selectedPath == null) return;
    const data = [];
    data.push(selectedPath);
    data.push(1);
    data.push(2);
    data.push(new Date().getTime());
    data.push(1000);
    data.push(fs.statSync(selectedPath).size);
    logger.info('start save cluster base info', data);
    CLUSTER_DATABASE.insertClusterBaseInfo(data);
}
