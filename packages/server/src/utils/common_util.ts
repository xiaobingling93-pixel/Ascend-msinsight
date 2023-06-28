import { SHA256 } from 'crypto-js';
import fs from 'fs';
import path from 'path';

/**
 * get traceId
 *
 * @param tid tid
 * @param pid pid
 * @returns traceId
 */
export function getTrackId(tid: number, pid: string): number {
    const str = pid + tid.toString();
    const hashDigest = SHA256(str);
    return hashDigest.words[0];
}

/**
 * 获取card id，即文件中rank_id属性
 *
 * @param filePath 文件路径
 */
export function parseCardID(filePath: string): number {
    let rankId = -1;
    try {
        const fd = fs.openSync(filePath, 'r');
        const buf = Buffer.alloc(128);
        fs.readSync(fd, buf, 0, 128, 0);
        const bufString = buf.toString('utf-8');
        console.log('bufString: ', bufString);
        const start = bufString.indexOf('{');
        const end = bufString.indexOf('}');
        const rankIDJsonString = bufString.substring(start, end + 1);
        const rank = JSON.parse(rankIDJsonString);
        rankId = rank.rank_id;
    } catch (err) {
        console.log(err);
    }
    return rankId;
}

/**
 * get database path by rankId
 *
 * @param filePath
 * @param rankId
 */
export function getDbPath(filePath: string, rankId: number): string {
    return './' + path.basename(filePath, '.json') + '_' + String(rankId) + '.db';
}
