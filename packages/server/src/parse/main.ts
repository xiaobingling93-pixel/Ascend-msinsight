import fs from 'fs';
import { ThreadPool } from './threadPool';
import { Table } from './table';
import { ParseMessage } from './parser_worker';

const defaultReadSize = 1024 * 1024 * 50;
const threadPool = new ThreadPool('./dist/parse/parser_worker.js');

const isRunTest = false;
if (isRunTest) {
    const dbPath = './trace_view.db';
    const filePath = 'assets/trace_view.json';
    parse(filePath, dbPath);
}

export async function parse(filePath: string, dbPath: string): Promise<void> {
    const start = new Date().getTime();
    console.log(`parse start. file: ${filePath}. dbPath: ${dbPath}`);
    const table = new Table(dbPath);
    await table.createTable();
    return new Promise((resolve, reject) => {
        threadPool.setAllTaskFinishCallback(async (data: any) => {
            await table.creatIndex();
            await table.updateDepth();
            await table.close();
            console.log('parse end. time:', new Date().getTime() - start);
            resolve();
        });
        const fileSize = fs.statSync(filePath).size;
        let readPosition = 0;
        fs.open(filePath, 'rs', (err, fd) => {
            if (err) {
                console.log(err);
            }
            while (readPosition < fileSize) {
                if (readPosition + defaultReadSize >= fileSize) {
                    console.log(`get read size. readPosition:${readPosition}, readSize:${fileSize - readPosition - 1}`);
                    threadPool.addTask({ filePath, dbPath, readPosition, readSize: fileSize - readPosition - 1 } as ParseMessage);
                    break;
                } else {
                    const data = getReadSize(fd, readPosition);
                    console.log(`get read size. readPosition:${data.readPosition}, readSize:${data.readSize}`);
                    threadPool.addTask({ filePath, dbPath, readPosition: data.readPosition, readSize: data.readSize });
                    readPosition = data.readPosition + data.readSize + 2;
                }
            }
        });
    });
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
