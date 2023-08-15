/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { tableMap } from '../database/tableManager';
import * as readline from 'readline';
import fs from 'fs';
import { mapperToKernelDetail } from '../utils/mapper_util';
import { getLoggerByName } from '../logger/loggger_configure';

const logger = getLoggerByName('parseKernelDetail', 'info');

export function parseKernelDetail(rankId: string, filePathArr: string[], callback?: (rankId: string, err?: Error) => void): void {
    logger.info('find kernel detail files:{},rankId:{}', filePathArr, rankId);
    if (!tableMap.has(rankId)) {
        if (callback) {
            callback(rankId, new Error('no rank Id:' + rankId));
        }
    }
    const table = tableMap.get(rankId);
    if (table === undefined) return;
    let count = 0;
    filePathArr.forEach(filePath => {
        logger.log('import kernel detail file start, filePath:{}', filePath);
        const stream = fs.createReadStream(filePath);
        const rl = readline.createInterface({
            input: stream,
            crlfDelay: 0,
        });
        rl.on('line', (line) => {
            // 处理每一行数据
            const regex = /,(?=(?:[^"]*"{2,3}[^"]*"{3,4})*(?![^"]*"))/;
            const arr = line.split(regex);
            if (arr.length >= 13) {
                const kernelDetail = mapperToKernelDetail(arr);
                table.insertKernelDetail(kernelDetail);
            } else {
                logger.info('split arr is less 13', line, ' arr:', arr);
            }
            count++;
        });
        rl.on('close', () => {
            table.saveLastKernelData();
            // 读取完成
            logger.log('import kernel detail file end, count:{}', count);
        });
    });
}
