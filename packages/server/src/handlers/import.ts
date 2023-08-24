import * as fs from 'fs';
import * as path from 'path';
import { Matchs, parseCardID } from '../utils/common_util';
import { parse } from '../parse/main';
import { Client } from '../types';
import { queryUnitsMetadata } from '../query/unitMetadataHandler';
import { exec } from 'child_process';
import * as os from 'os';
import { promisify } from 'util';
import { getLoggerByName } from '../logger/loggger_configure';
import { waitResetComplete } from './reset';

const logger = getLoggerByName('import', 'info');
const execute = promisify(exec);
function findJsonFiles(dir: string, traceViewJsonPaths: string[], depth: number): void {
    const stats = fs.statSync(dir);
    if (stats.isFile()) {
        if (isJsonValid(path.basename(dir))) {
            traceViewJsonPaths.push(dir);
        }
        return;
    }
    if (depth > 7) { return; } // 控制递归深度
    const files = fs.readdirSync(dir);
    for (const file of files) {
        const filePath = path.join(dir, file);
        const stats = fs.statSync(filePath);
        if (stats.isDirectory()) {
            findJsonFiles(filePath, traceViewJsonPaths, depth + 1);
        } else if (isJsonValid(file)) {
            traceViewJsonPaths.push(filePath);
        }
    }
}

const isJsonValid = (fileName: string): boolean => {
    const msprofPattern = Matchs.msprof;
    const validTraceView = fileName === Matchs.trace_view;
    const validMsprof = fileName.match(msprofPattern) !== null;
    return validMsprof || validTraceView;
};

async function selectFolderWindows(): Promise<string> {
    const script = '[Console]::OutputEncoding = [System.Text.Encoding]::UTF8; Add-Type -AssemblyName System.Windows.Forms; $dialog = New-Object System.Windows.Forms.FolderBrowserDialog;$result = $dialog.ShowDialog(); if ($result -eq “OK”) { $dialog.SelectedPath }';
    try {
        const { stdout } = await execute(`PowerShell -Command "${script}"`);
        return stdout.trim();
    } catch (error) {
        logger.error(error);
        return '';
    }
}

async function selectFolderLinux(): Promise<string> {
    try {
        const { stdout } = await execute('zenity --file-selection --directory');
        return stdout.trim();
    } catch (error) {
        logger.error(error);
        return '';
    }
}

async function selectFolder(): Promise<string | null> {
    if (os.platform() === 'win32') {
        const result = await selectFolderWindows();
        return result;
    } else if (os.platform() === 'linux') {
        return await selectFolderLinux();
    }
    return null;
}

async function findJsons(path: string): Promise<string[]> {
    const traceJsonPaths: string[] = [];
    try {
        if (path === 'browser') {
            await selectFolder().then(folderPath => {
                if (folderPath === null) {
                    return traceJsonPaths;
                }
                findJsonFiles(folderPath, traceJsonPaths, 0);
                return traceJsonPaths;
            });
        } else {
            findJsonFiles(path, traceJsonPaths, 0);
        }
    } catch (error) {
        logger.error(error);
    }
    filterJsonPaths(traceJsonPaths);
    return traceJsonPaths;
}

const filterJsonPaths = (traceJsonPaths: string[]): void => {
    const resultFolders = new Set();
    traceJsonPaths.forEach(filePath => {
        const ascendOutputFolder = getMatchedPathSegment(filePath, Matchs.ascend_output);
        if (ascendOutputFolder !== null) {
            resultFolders.add(path.dirname(ascendOutputFolder));
        }
    });

    for (let i = traceJsonPaths.length - 1; i >= 0; i--) {
        const filePath = traceJsonPaths[i];
        if (filePath.match(Matchs.msprof)) {
            const ProfFolder = getMatchedPathSegment(filePath, Matchs.PROF);
            if (ProfFolder !== null && resultFolders.has(path.dirname(ProfFolder))) {
                traceJsonPaths.splice(i, 1);
            }
        }
    }
};

const getMatchedPathSegment = (filePath: string, regex: RegExp): string | null => {
    const pathSegments = filePath.split(path.sep);
    let matchedPath = '';
    for (let i = 0; i < pathSegments.length; i++) {
        matchedPath = path.join(matchedPath, pathSegments[i]);
        if (regex.test(pathSegments[i])) {
            return matchedPath;
        }
    }
    return null;
};

type CardInfo = {
    cardName: string;
    rankId: string;
    result: boolean;
};

export const importHandler = async (req: { path: string }, client: Client): Promise<Record<string, unknown>> => {
    const traceViewJsonPaths = await findJsons(req.path);
    const importedRankIdSet = client.shadowSession.importedRankIdSet;
    const extremumTimestamp = client.shadowSession.extremumTimestamp;
    const result: CardInfo[] = [];
    await waitResetComplete();
    for (const traceViewJsonPath of traceViewJsonPaths) {
        const rankId = parseCardID(traceViewJsonPath);
        if (importedRankIdSet.has(rankId)) {
            continue;
        }
        const cardInfo: CardInfo = { cardName: rankId.toString(), rankId, result: true };
        result.push(cardInfo);
        parse(traceViewJsonPath, rankId, (rankId, err) => {
            if (err) {
                // this to send parse file error message
                logger.error(`parse error. ${err.message}`);
                cardInfo.result = false;
                client?.notify('parse/fail', { rankId, errorMsg: err.message });
                return;
            }
            // this to send parse file success message
            queryUnitsMetadata(rankId).then((queryResult) => {
                let startTimeUpdated = false;
                if (extremumTimestamp.minTimestamp > queryResult.extremumTimestamp.minTimestamp) {
                    extremumTimestamp.maxTimestamp = extremumTimestamp.maxTimestamp + extremumTimestamp.minTimestamp - queryResult.extremumTimestamp.minTimestamp;
                    extremumTimestamp.minTimestamp = queryResult.extremumTimestamp.minTimestamp;
                    startTimeUpdated = true;
                }
                extremumTimestamp.maxTimestamp = Math.max(queryResult.extremumTimestamp.maxTimestamp - extremumTimestamp.minTimestamp, extremumTimestamp.maxTimestamp);
                client?.notify('parse/success', { unit: queryResult.insightMetaData, startTimeUpdated, maxTimeStamp: extremumTimestamp.maxTimestamp });
            });
            logger.info('send notify rankId parse end. ', rankId);
        });
        importedRankIdSet.add(rankId);
    }
    return { result };
};
