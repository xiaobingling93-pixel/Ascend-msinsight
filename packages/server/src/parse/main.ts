import fs from 'fs';
import path from 'path';
import { ThreadPool } from './threadPool';
import { Table } from '../database/table';
import { tableMap } from '../database/tableManager';
import { EndMessage, ParseMessage } from './parser_worker';

const defaultReadSize = 1024 * 1024 * 50;
const parseTaskCount = new Map<string, number>(); // rankId, task count
const callbackMap = new Map<string, Function>(); // rankId, callback
const threadPool = new ThreadPool('./dist/parse/parser_worker.js', parseFileEnd);

export function parse(filePath: string, rankId: string, callback?: (err: Error | null, rankId: string) => void): void {
    if (tableMap.has(rankId)) {
        if (callback) {
            callback(new Error('repeat rank Id'), rankId);
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
            console.log(err);
        }
        while (readPosition < fileSize) {
            if (readPosition + defaultReadSize >= fileSize) {
                console.log(`get read size. rankId:${rankId}, readPosition:${readPosition}, readSize:${fileSize - readPosition - 1}`);
                threadPool.addTask({ rankId, filePath, dbPath, readPosition, readSize: fileSize - readPosition - 1 } as ParseMessage);
                taskCount++;
                break;
            } else {
                const data = getReadSize(fd, readPosition);
                console.log(`get read size. rankId:${rankId}, readPosition:${data.readPosition}, readSize:${data.readSize}`);
                taskCount++;
                threadPool.addTask({ rankId, filePath, dbPath, readPosition: data.readPosition, readSize: data.readSize });
                readPosition = data.readPosition + data.readSize + 2;
            }
        }
        parseTaskCount.set(rankId, taskCount);
        fs.close(fd);
    });
}

async function parseFileEnd(message: EndMessage): Promise<string> {
    if (!tableMap.has(message.rankId) || !parseTaskCount.has(message.rankId)) {
        console.log(`can not find rankId, ${message.rankId}`);
        return '';
    }
    const unfinishedTaskCount = parseTaskCount.get(message.rankId) as number;
    console.log(`parseFileEnd. rankId:${message.rankId}, count: ${unfinishedTaskCount}`);
    if (unfinishedTaskCount - 1 === 0) {
        parseTaskCount.delete(message.rankId);
        const table = tableMap.get(message.rankId) as Table;
        await table.creatIndex();
        await table.updateDepth();
        await table.close();
        console.log(`parse end. rankId:${message.rankId}`);
        callbackMap.get(message.rankId)?.(null, message.rankId);
        callbackMap.delete(message.rankId);
        return message.rankId;
    } else {
        parseTaskCount.set(message.rankId, unfinishedTaskCount - 1);
        return '';
    }
}

function getReadSize(fd: number, start: number): { readPosition: number; readSize: number } {
    const buf = Buffer.alloc(1024 * 10);
    fs.readSync(fd, buf, 0, 1024, start);
    let offset = buf.toString('utf-8').indexOf('{');
    if (offset < 0) {
        console.log('no find {');
        return { readPosition: 0, readSize: 0 };
    }
    const readPosition = start + offset;
    fs.readSync(fd, buf, 0, buf.length, readPosition + defaultReadSize);
    offset = buf.toString('utf-8').indexOf('}, {');
    if (offset < 0) {
        console.log('no find }, {');
        return { readPosition: 0, readSize: 0 };
    }
    const readSize = defaultReadSize + offset + 1;
    return { readPosition, readSize };
}

function getDbPath(filePath: string, rankId: string): string {
    return './' + path.basename(filePath, '.json') + '_' + rankId + '.db';
}
