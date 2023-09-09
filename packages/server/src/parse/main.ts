import fs from 'fs';
import { getDbPath } from '../utils/common_util';
import { ThreadPool } from './threadPool';
import { Table } from '../database/table';
import { tableMap } from '../database/tableManager';
import { EndMessage } from './parser_worker';

const defaultReadSize = 1024 * 1024 * 50;
const parseTaskCount = new Map<string, number>(); // rankId, task count
const callbackMap = new Map<string, Function>(); // rankId, callback
const threadPool = new ThreadPool(parseWorkerEnd);
let depthTaskCount = 0;

export function parse(filePath: string, rankId: string, callback?: (rankId: string, err?: Error) => void): void {
    if (callback) {
        callbackMap.set(rankId, callback);
    }
    if (tableMap.has(rankId)) {
        parseCallback(rankId, new Error('Repeat rank id'));
    }
    const dbPath = getDbPath(filePath, rankId);
    console.log(`Save to db. ${dbPath}`);
    if (dbPath.length === 0) {
        parseCallback(rankId, new Error('Failed to creat db path.'));
        return;
    }
    const table = new Table(dbPath);
    table.createTable().then(() => parseFile(filePath, dbPath, rankId));
    tableMap.set(rankId, table);
}

function parseFile(filePath: string, dbPath: string, rankId: string): void {
    const fileSize = fs.statSync(filePath).size;
    let readPosition = 0;
    let taskCount = 0;
    fs.open(filePath, 'rs', (err, fd) => {
        if (err) {
            parseCallback(rankId, err);
            return;
        }
        while (readPosition < fileSize) {
            const data = getReadSize(fd, readPosition, fileSize);
            if (data.readSize === 0) {
                console.log('Failed to split file.');
                continue;
            }
            console.log(`get read size. rankId:${rankId}, readPosition:${data.readPosition}, readSize:${data.readSize}`);
            taskCount++;
            threadPool.addTask({ rankId, filePath, dbPath, readPosition: data.readPosition, readSize: data.readSize });
            readPosition = data.readPosition + data.readSize + 2;
        }
        parseTaskCount.set(rankId, taskCount);
        fs.close(fd, () => { });
    });
}

async function parseWorkerEnd(message: EndMessage): Promise<void> {
    if (!tableMap.has(message.rankId) || !parseTaskCount.has(message.rankId)) {
        console.log(`can not find rankId, ${message.rankId}`);
        return;
    }
    const unfinishedTaskCount = parseTaskCount.get(message.rankId) as number;
    console.log(`parseFileEnd. rankId:${message.rankId}, count: ${unfinishedTaskCount}`);
    if (unfinishedTaskCount - 1 === 0) {
        depthTaskCount++;
        console.log('depthTaskCount++:', depthTaskCount);
        const table = tableMap.get(message.rankId) as Table;
        await table.creatIndex();
        await table.updateDepth();
        console.log(`parse end. rankId:${message.rankId}`);
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
        console.log('no find {');
        return { readPosition: 0, readSize: 0 };
    }
    const readPosition = start + offset;
    let readSize: number;
    if (readPosition + defaultReadSize >= fileSize) {
        fs.readSync(fd, buf, 0, buf.length, fileSize - buf.length);
        offset = buf.toString('utf-8').indexOf(']');
        readSize = fileSize - readPosition - (buf.length - offset);
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
