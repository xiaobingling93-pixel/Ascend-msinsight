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

export function parse(filePath: string, rankId: string, callback?: (rankId: string, err?: Error) => void): void {
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
    table.createTable().then(() => parseFile(filePath, dbPath, rankId));
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
        fs.close(fd);
    });
}

async function parseWorkerEnd(message: EndMessage): Promise<void> {
    if (!tableMap.has(message.rankId) || !parseTaskCount.has(message.rankId)) {
        console.log(`can not find rankId, ${message.rankId}`);
    }
    const unfinishedTaskCount = parseTaskCount.get(message.rankId) as number;
    console.log(`parseFileEnd. rankId:${message.rankId}, count: ${unfinishedTaskCount}`);
    if (unfinishedTaskCount - 1 === 0) {
        parseTaskCount.delete(message.rankId);
        const table = tableMap.get(message.rankId) as Table;
        await table.creatIndex();
        await table.updateDepth();
        console.log(`parse end. rankId:${message.rankId}`);
        parseCallback(message.rankId);
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
