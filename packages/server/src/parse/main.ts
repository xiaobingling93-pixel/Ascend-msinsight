import fs from 'fs';
import path from 'path';
import { ThreadPool } from './threadPool';
import { Table } from '../database/table';
import { tableMap } from '../database/tableManager';
import { EndMessage, ParseMessage } from './parser_worker';

const defaultReadSize = 1024 * 1024 * 50;
const parseTaskCount = new Map<string, number>(); // rankId, task count
const threadPool = new ThreadPool('./dist/parse/parser_worker.js');

export async function parse(filePath: string, rankId: string): Promise<void> {
    return new Promise((resolve, reject) => {
        if (tableMap.has(rankId)) {
            reject(new Error('repeat rank Id'));
        }
        const start = new Date().getTime();
        const dbPath = getDbPath(filePath, rankId);
        const table = new Table(dbPath);
        tableMap.set(rankId, table);
        threadPool.setTaskFinishCallback((message: EndMessage) => {
            parseFileEnd(message).then(rankId => {
                console.log(`parse end. rankId:${rankId}, time:${new Date().getTime() - start}`);
            });
        });
        table.createTable().then(() => parseFile(filePath, dbPath, rankId));
    });
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
                console.log(`get read size. readPosition:${readPosition}, readSize:${fileSize - readPosition - 1}`);
                threadPool.addTask({ rankId, filePath, dbPath, readPosition, readSize: fileSize - readPosition - 1 } as ParseMessage);
                taskCount++;
                break;
            } else {
                const data = getReadSize(fd, readPosition);
                console.log(`get read size. readPosition:${data.readPosition}, readSize:${data.readSize}`);
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
        // reject(new Error('repeat rank Id'));
    }
    const unfinishedTaskCount = parseTaskCount.get(message.rankId) as number;
    if (unfinishedTaskCount - 1 === 0) {
        parseTaskCount.delete(message.rankId);
        const table = tableMap.get(message.rankId) as Table;
        await table.creatIndex();
        await table.updateDepth();
        await table.close();
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
    // console.log(buf.toString(), ' ', offset);
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
