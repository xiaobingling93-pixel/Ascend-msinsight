/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { Client } from '../types';
import { getLoggerByName } from '../logger/loggger_configure';
import fs from 'fs';
import { join } from 'path';
import { parseFile } from './import';

const logger = getLoggerByName('import', 'info');

export type ImportRequest = {
    index: number;
    count: number;
    name: string;
    path: string;
    buffer: any;
    isInFolder: boolean;
    isLast: boolean;
    slice: {
        isSliced: boolean;
        index: number;
        count: number;
    };
};

function mkdirByFilePath(realPath: string): void {
    if (!fs.existsSync(join(__dirname, 'download', realPath))) {
        fs.mkdir(join(__dirname, 'download', realPath), { recursive: true }, (err, path) => {
            if (err) {
                return logger.error('创建文件夹失败');
            } else {
                logger.info('文件夹创建成功');
            }
        });
    }
}

export const towingImportHandler = async (request: ImportRequest, client: Client): Promise<Record<string, unknown>> => {
    logger.info('接收请求，开始下载文件');
    const { buffer, isLast, name, path, isInFolder } = request;
    const realPath = path.split(name)[0].toString();
    let data = Buffer.alloc(0);
    data = Buffer.concat([ data, buffer ], data.length + buffer.length);
    const downLoadPath = isInFolder ? join(__dirname, 'download') : join(__dirname, 'download', realPath);
    mkdirByFilePath(realPath);
    // 需要接收一个flag 来识别是否为切片文件
    const isSliced: boolean = request.slice.isSliced;
    const sliceIndex: number = request.slice.index;
    if (isSliced && sliceIndex === 1 && fs.existsSync(join(downLoadPath, path))) {
        fs.unlinkSync(join(downLoadPath, path));
        logger.info('delete repeat file');
    }
    if (isSliced && sliceIndex > 1) {
        fs.appendFile(join(downLoadPath, path), data, function(err) {
            if (err) {
                return logger.error('文件追加失败');
            } else {
                logger.info('文件追加成功');
            }
        });
    } else {
        fs.writeFile(join(downLoadPath, path), data, { flag: 'w+' }, function(err) {
            if (err) {
                return logger.error('文件下载失败');
            } else {
                logger.info('文件下载成功');
            }
        });
    }

    if (isInFolder) {
        if (isLast) {
            logger.info('开始解析文件');
            return await parseFile(downLoadPath, client);
        }
    } else {
        if (sliceIndex === request.slice.count) {
            logger.info('开始解析文件');
            return await parseFile(downLoadPath, client);
        }
    }
    return { msg: '文件下载还在继续', isEnd: false };
};
