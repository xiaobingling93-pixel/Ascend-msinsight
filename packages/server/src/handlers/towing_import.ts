/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { Client } from '../types';
import { getLoggerByName } from '../logger/loggger_configure';
import { promises as fsPromises } from 'fs';
import { join } from 'path';

const logger = getLoggerByName('import', 'info');

export type ImportRequest = {
    file: any;
    name: string;
    path: string;
};

export type ImportResponse = {
    status: string;
    data: string;
};

async function saveFileFromBytes(fileContent: any, filePath: string): Promise<void> {
    try {
        await fsPromises.writeFile(join(__dirname, filePath), fileContent, { flag: 'w' });
        logger.info('文件下载成功');
    } catch (err) {
        logger.error(err);
    }
}

export const towingImportHandler = async (request: ImportRequest, client: Client): Promise<ImportResponse> => {
    logger.info('接收请求，开始下载文件');
    const fileContent = request.file;
    const arrayBuffer = objectToArrayBuffer(fileContent);
    const buffer = toBuffer(arrayBuffer);
    const fileName = request.name;
    const response: ImportResponse = { status: '', data: '' };
    await saveFileFromBytes(buffer, fileName);
    return response;
};

function objectToArrayBuffer(obj: any): ArrayBuffer {
    const str = JSON.stringify(obj);
    const buffer = new ArrayBuffer(str.length * 2);
    const view = new Uint16Array(buffer);

    for (let i = 0; i < str.length; i++) {
        view[i] = str.charCodeAt(i);
    }

    return buffer;
}

function toBuffer(arrayBuffer: ArrayBuffer): Buffer {
    const buffer = Buffer.alloc(arrayBuffer.byteLength);
    const view = new Uint8Array(arrayBuffer);
    for (let i = 0; i < buffer.length; ++i) {
        buffer[i] = view[i];
    }
    return buffer;
}
