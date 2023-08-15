import { SHA256 } from 'crypto-js';
import fs from 'fs';
import path from 'path';
import crypto from 'crypto';
import { getLoggerByName } from '../logger/loggger_configure';

const logger = getLoggerByName('common_util', 'info');

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
    logger.info('start parse CardID');
    let rankId = crypto.randomUUID() as string;
    try {
        const fd = fs.openSync(filePath, 'r');
        const buf = Buffer.alloc(128);
        fs.readSync(fd, buf, 0, 128, 0);
        const bufString = buf.toString('utf-8');
        logger.info('bufString: ', bufString);
        const start = bufString.indexOf('{');
        const end = bufString.indexOf('}');
        const rankIDJsonString = bufString.substring(start, end + 1);
        const rank = JSON.parse(rankIDJsonString);
        return String(rank.rank_id);
    } catch (err) {
        logger.error(err);
    }
    try {
        logger.info('try get dirname*2 of filePath.');
        rankId = path.basename(path.dirname(path.dirname(filePath)));
    } catch (error) {
        logger.error(error);
    }
    return rankId;
}

/**
 * 根据外层文件名获取rankId profiler_info_{}.json
 *
 * @param filePath 文件路径
 */
export function parseRankIdByFile(filePath: string): string {
    logger.info('start parse rank id');
    let rankId = crypto.randomUUID() as string;
    try {
        logger.info('try get dirname*2 of filePath.');
        const rankFile = findFilesByPath(path.dirname(path.dirname(filePath)), 'profiler_info_[0-9]{1,}.json');
        if (rankFile.length > 0) {
            rankId = path.basename(rankFile[0])
                .replace('profiler_info_', '')
                .replace('.json', '');
        } else {
            rankId = path.basename(path.dirname(path.dirname(filePath)));
        }
    } catch (error) {
        logger.error('parseRankIdByFile :', error);
    }
    return rankId;
}

export function findJsonFiles(dir: string, matchedFilePaths: string[], depth: number, fileFormatter: RegExp): void {
    if (depth > 5) return; // 控制递归深度
    const files = fs.readdirSync(dir);
    for (const file of files) {
        const filePath = path.join(dir, file);
        const stats = fs.statSync(filePath);
        if (stats.isDirectory()) {
            findJsonFiles(filePath, matchedFilePaths, depth + 1, fileFormatter);
            // } else if (file === 'trace_view.json') {
        } else if (fileFormatter.test(file)) {
            matchedFilePaths.push(filePath);
        }
    }
}

export function findFilesByPath(path: string | null, fileNameFormatter: string): string[] {
    const filePaths: string[] = [];
    if (path === null) {
        return filePaths;
    }
    const fileFormatter = new RegExp(fileNameFormatter);
    try {
        findJsonFiles(path, filePaths, 0, fileFormatter);
    } catch (error) {
        logger.error(error);
    }
    return filePaths;
}

/**
 * get database path by rankId
 *
 * @param filePath
 * @param rankId
 */
export function getDbPath(filePath: string[], rankId: string): string {
    if (filePath.length > 1) {
        return './' + rankId + '.db';
    }
    return './' + path.basename(filePath[0], '.json') + '_' + rankId + '.db';
}

export function getFolderSize(folderPath: string): number {
    let totalSize = 0;
    const files = fs.readdirSync(folderPath);
    files.forEach(function(file) {
        const filePath = path.join(folderPath, file);
        const stats = fs.statSync(filePath);
        if (stats.isFile()) {
            totalSize += stats.size;
        } else if (stats.isDirectory()) {
            totalSize += getFolderSize(filePath);
        }
    });
    return totalSize;
}

export function isJsonStr(str: string): boolean {
    try {
        JSON.parse(str);
        return true;
    } catch (e) {
        logger.error('parse json str is err:', str);
        return false;
    }
}
