/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { Client } from '../types';
import { getLoggerByName } from '../logger/loggger_configure';
import fs from 'fs';
import { join } from 'path';
import { parseFile } from './import';
import os from 'os';

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

function splitPathByOs(isInFolder: boolean, rootName: string, path: string): string {
    if (isInFolder && os.platform() === 'win32') {
        rootName = path.split('/')[0].toString();
    }
    if (isInFolder && os.platform() === 'linux') {
        rootName = path.split('\\')[0].toString();
    }
    return rootName;
}

export const towingImportHandler = async (request: ImportRequest, client: Client): Promise<Record<string, unknown>> => {
    logger.info('接收请求，开始下载文件');
    const { buffer, isLast, name, path, isInFolder } = request;
    const realPath = path.split(name)[0].toString();
    let rootName = '';
    rootName = splitPathByOs(isInFolder, rootName, path);
    let data = Buffer.alloc(0);
    data = Buffer.concat([ data, buffer ], data.length + buffer.length);
    const parentFilePath = join(__dirname, 'download', realPath);
    const downLoadPath = isInFolder ? join(__dirname, 'download', rootName) : parentFilePath;
    mkdirByFilePath(parentFilePath);
    // 需要接收一个flag 来识别是否为切片文件
    const isSliced: boolean = request.slice.isSliced;
    const sliceIndex: number = request.slice.index;
    const filePath = join(__dirname, 'download', path);
    if (isSliced && sliceIndex === 1 && fs.existsSync(filePath)) {
        fs.unlinkSync(filePath);
        logger.info('delete repeat file');
    }
    if (isSliced && sliceIndex > 1) {
        fs.appendFile(filePath, data, function(err) {
            if (err) {
                return logger.error('文件追加失败');
            } else {
                logger.info('文件追加成功');
            }
        });
    } else {
        fs.writeFile(filePath, data, { flag: 'w+' }, function(err) {
            if (err) {
                return logger.error('文件下载失败');
            } else {
                logger.info('文件下载成功');
            }
        });
    }

    if (isInFolder) {
        if (isLast && (!isSliced || (isSliced && sliceIndex === request.slice.count))) {
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

function mkdirByFilePath(parentFilePath: string): void {
    if (!fs.existsSync(parentFilePath)) {
        fs.mkdirSync(parentFilePath, { recursive: true });
        logger.info(parentFilePath, '文件夹创建成功');
    }
}
