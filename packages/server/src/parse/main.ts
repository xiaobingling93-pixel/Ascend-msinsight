import fs from 'fs';
import { getDbPath } from '../utils/common_util';
import { ThreadPool } from './threadPool';
import { Table } from '../database/table';
import { tableMap } from '../database/tableManager';
import { EndMessage } from './parser_worker';
import { getLoggerByName } from '../logger/loggger_configure';
import path from 'path';
import { importKernelDetail } from '../handlers/import';

const defaultReadSize = 1024 * 1024 * 50;
const parseTaskCount = new Map<string, number>(); // rankId, task count
const callbackMap = new Map<string, Function>(); // rankId, callback
const threadPool = new ThreadPool(parseWorkerEnd);
let depthTaskCount = 0;

const logger = getLoggerByName('main', 'info');

export function parse(filePath: string[], rankId: string, callback?: (rankId: string, err?: Error) => void): void {
    if (tableMap.has(rankId)) {
        if (callback) {
            callback(rankId, new Error('repeat rank Id'));
        }
    }
    const dbPath = getDbPath(filePath, rankId);
    const table = new Table(dbPath);
    tableMap.set(rankId, table);
    if (callback) {
        callbackMap.set(rankId, callback);
    }
    table.createTable()
        .then(() => parseFile(filePath, dbPath, rankId))
        .then(() => importKernelDetail(path.dirname(filePath[0]), rankId))
        .then(() => table.close);
}

function parseFile(filePathArr: string[], dbPath: string, rankId: string): void {
    let taskCount = 0;
    for (const filePath of filePathArr) {
        const fileSize = fs.statSync(filePath).size;
        let readPosition = 0;
        logger.log(`FetchSize:${fileSize},readPosition:${readPosition}`);
        fs.open(filePath, 'rs', (err, fd) => {
            if (err) {
                parseCallback(rankId, err);
                return;
            }
            while (readPosition < fileSize) {
                const data = getReadSize(fd, readPosition, fileSize);
                if (data.readSize === 0) {
                    logger.log('Failed to split file.');
                    continue;
                }
                logger.log(`get read size. rankId:${rankId}, readPosition:${data.readPosition}, readSize:${data.readSize}`);
                taskCount++;
                threadPool.addTask({ rankId, filePath, dbPath, readPosition: data.readPosition, readSize: data.readSize });
                readPosition = data.readPosition + data.readSize + 2;
            }
            parseTaskCount.set(rankId, taskCount);
            fs.close(fd, () => { });
        });
    }
}

async function parseWorkerEnd(message: EndMessage): Promise<void> {
    if (!tableMap.has(message.rankId) || !parseTaskCount.has(message.rankId)) {
        logger.log(`can not find rankId, ${message.rankId}`);
        return;
    }
    const unfinishedTaskCount = parseTaskCount.get(message.rankId) as number;
    logger.log(`parseFileEnd. rankId:${message.rankId}, count: ${unfinishedTaskCount}`);
    if (unfinishedTaskCount - 1 === 0) {
        depthTaskCount++;
        console.log('depthTaskCount++:', depthTaskCount);
        const table = tableMap.get(message.rankId) as Table;
        await table.creatIndex();
        await table.updateDepth();
        await table.updateOverlapDuration();
        logger.log(`parse end. rankId:${message.rankId}`);
        parseCallback(message.rankId);
        parseTaskCount.delete(message.rankId);
        depthTaskCount--;
        console.log('depthTaskCount--:', depthTaskCount);
    } else {
        parseTaskCount.set(message.rankId, unfinishedTaskCount - 1);
    }
}

function getReadSize(fd: number, start: number, fileSize: number): { readPosition: number; readSize: number } {
    const buf = Buffer.alloc(1024 * 10);
    fs.readSync(fd, buf, 0, 1024, start);
    let offset = buf.toString('utf-8').indexOf('{');
    if (offset < 0) {
        logger.log('no find {');
        return { readPosition: 0, readSize: 0 };
    }
    const readPosition = start + offset;
    let readSize: number;
    if (readPosition + defaultReadSize >= fileSize) {
        const tempStartPosition = fileSize - buf.length > 0 ? fileSize - buf.length : 0;
        fs.readSync(fd, buf, 0, buf.length, tempStartPosition);
        offset = buf.toString('utf-8').indexOf(']');
        readSize = fileSize - readPosition - (Math.min(buf.length, fileSize) - offset);
    } else {
        fs.readSync(fd, buf, 0, buf.length, readPosition + defaultReadSize);
        offset = buf.toString('utf-8').indexOf('}, {');
        readSize = defaultReadSize + offset + 1;
    }
    return { readPosition, readSize };
}

function parseCallback(rankId: string, err?: Error): void {
    callbackMap.get(rankId)?.(rankId, err);
    callbackMap.delete(rankId);
}

export async function terminateParse(): Promise<void> {
    return new Promise(resolve => {
        // 删除未解析完的任务
        console.log('terminateParse');
        for (const [ key, value ] of parseTaskCount) {
            if (value !== 1) {
                parseTaskCount.delete(key);
            }
        }
        threadPool.terminateAllTask().then(async () => {
            parseTaskCount.clear();
            callbackMap.clear();
            while (true) {
                if (depthTaskCount === 0) {
                    console.log('terminateAllTask');
                    resolve();
                    break;
                }
                await new Promise((resolve) => setTimeout(resolve, 1000));
            }
        });
    });
}
