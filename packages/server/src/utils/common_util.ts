import { SHA256 } from 'crypto-js';
import fs from 'fs';
import path from 'path';
import crypto from 'crypto';

export const Matchs = {
    ascend_pt: /.*_ascend_pt/,
    msprof: /msprof.*\.json/,
    trace_view: 'trace_view.json',
};

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
export function parseCardID(filePath: string): string {
    console.log('start parse CardID');
    let rankId = crypto.randomUUID() as string;
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
        return String(rank.rank_id);
    } catch (err) {
        console.error(err);
    }
    try {
        rankId = getRankIdFromPath(filePath);
    } catch (error) {
        console.error(error);
    }
    return rankId;
}

const getRankIdFromPath = (filePath: string): string => {
    console.log('try get disPlayName of filePath.');
    const match = filePath.match(Matchs.ascend_pt);
    if (match) {
        if (filePath.endsWith(Matchs.trace_view)) {
            return path.basename(match[0]);
        } else if (filePath.match(Matchs.msprof)) {
            const devicePath = path.dirname(path.dirname(filePath));
            return path.basename(path.dirname(devicePath)) + path.basename(devicePath);
        }
    }
    return '';
};

/**
 * get database path by rankId
 *
 * @param filePath
 * @param rankId
 */
export function getDbPath(filePath: string, rankId: string): string {
    try {
        if (!fs.existsSync('database')) {
            fs.mkdirSync('database');
        }
        return path.resolve(path.join('database', path.basename(filePath, '.json') + '_' + rankId + '.db'));
    } catch (e) {
        console.log('Can not creat the folder. error:', (e as Error).message);
        return '';
    }
}
