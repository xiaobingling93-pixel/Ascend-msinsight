import * as fs from 'fs';
import * as path from 'path';
import { Matchs, findFilesByPath, parseRankIdByFile } from '../utils/common_util';
import { parse } from '../parse/main';
import { Client } from '../types';
import { queryUnitsMetadata } from '../query/unitMetadataHandler';
import { exec } from 'child_process';
import * as os from 'os';
import { promisify } from 'util';
import { getLoggerByName } from '../logger/loggger_configure';
import { waitResetComplete } from './reset';
import { parseKernelDetail } from '../parse/kernel_detail.parser';
import {
    parseCommunicationFile,
    parseCommunicationMatrixFile,
    parseStepStatisticsFile,
    saveClusterBaseInfo,
} from '../parse/communication_parser';
import { CLUSTER_DATABASE } from '../database/tableManager';

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

export async function selectFolder(): Promise<string> {
    if (os.platform() === 'win32') {
        return await selectFolderWindows();
    } else if (os.platform() === 'linux') {
        return await selectFolderLinux();
    }
    return '';
}

async function findJsons(path: string): Promise<string[]> {
    const traceJsonPaths: string[] = [];
    if (path == null) {
        return traceJsonPaths;
    }
    try {
        findJsonFiles(path, traceJsonPaths, 0);
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

export function splitRankFile(matchedFilePaths: string[]): Map<string, string[]> {
    const resultMap = new Map<string, string[]>();
    matchedFilePaths.forEach(filePath => {
        const tempRankId = parseRankIdByFile(filePath);
        const tempArr = resultMap.get(tempRankId);
        if (tempArr === undefined) {
            resultMap.set(tempRankId, [filePath]);
        } else {
            tempArr.push(filePath);
        }
    });
    return resultMap;
}

type CardInfo = {
    cardName: string;
    rankId: string;
    result: boolean;
};

export const importHandler = async (req: { path: string }, client: Client): Promise<Record<string, unknown>> => {
    let selectedFolder = req.path;
    selectedFolder = req.path === 'browser' ? await selectFolder() : selectedFolder;
    const timelineJsonPaths = await findJsons(selectedFolder);
    const importedRankIdSet = client.shadowSession.importedRankIdSet;
    const extremumTimestamp = client.shadowSession.extremumTimestamp;
    const timeLineJsonFileMap = splitRankFile(timelineJsonPaths);
    const scene = 'train';
    const result: {cards: CardInfo[]; steps: string[]; scene: string} = { cards: [], steps: [], scene };
    await waitResetComplete();
    for (const rankId of timeLineJsonFileMap.keys()) {
        const fileArr = timeLineJsonFileMap.get(rankId);
        if (importedRankIdSet.has(rankId) || fileArr === undefined) continue;
        const cardInfo: CardInfo = { cardName: rankId.toString(), rankId, result: true };
        result.cards.push(cardInfo);
        // 按卡解析文件
        parse(fileArr, rankId, (rankId, err) => {
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
                if (queryResult.extremumTimestamp.minTimestamp !== null && queryResult.extremumTimestamp.maxTimestamp !== null) {
                    if (extremumTimestamp.minTimestamp > queryResult.extremumTimestamp.minTimestamp) {
                        extremumTimestamp.maxTimestamp = extremumTimestamp.maxTimestamp + extremumTimestamp.minTimestamp - queryResult.extremumTimestamp.minTimestamp;
                        extremumTimestamp.minTimestamp = queryResult.extremumTimestamp.minTimestamp;
                        startTimeUpdated = true;
                    }
                    extremumTimestamp.maxTimestamp = Math.max(queryResult.extremumTimestamp.maxTimestamp - extremumTimestamp.minTimestamp, extremumTimestamp.maxTimestamp);
                }
                client?.notify('parse/success', { unit: queryResult.insightMetaData, startTimeUpdated, maxTimeStamp: extremumTimestamp.maxTimestamp });
            });
            logger.info('send notify rankId parse end. ', rankId);
        });
        importedRankIdSet.add(rankId);
    }
    // 多卡场景才处理
    if (selectedFolder !== null && scene === 'train' && result.cards.length > 1) {
        await execClusterScenario(selectedFolder, client);
    }
    return result;
};

async function execClusterScenario(selectedFolder: string, client: Client): Promise<void> {
    logger.info('generate train cluster tables');
    await execClusterPython(selectedFolder);
    await CLUSTER_DATABASE.createClusterTable();
    // import communication data
    importCommunication(selectedFolder, client);
    // write base info table
    saveClusterBaseInfo(selectedFolder);
    // import step statistics data
    importStepStatistics(selectedFolder);
    // import communication matrix data
    importCommunicationMatrix(selectedFolder, client);
}

function execClusterPython(folder: string): void {
    if (!fs.existsSync(folder + '/cluster_analysis_output') &&
        fs.existsSync('resource/cluster_analyse/cluster_analysis.py')) {
        logger.info('can not find dir cluster_analysis_output, start execute merge cluster python script');
        exec(`python resource/cluster_analyse/cluster_analysis.py -d ${folder}`,
            (error, stdout, stderr) => {
                if (error) {
                    logger.error(`exec error: ${error}`);
                    return;
                }
                logger.info(`stdout: ${stdout}`);
                logger.info(`stderr: ${stderr}`);
            });
    }
}
export const importCommunication = (selectedPath: string, client: Client): void => {
    const communicationFileArr = findFilesByPath(selectedPath, 'cluster_communication.json');
    if (communicationFileArr.length === 0) {
        logger.info('communication file is not found.');
        return;
    }
    parseCommunicationFile(communicationFileArr, client);
};

export const importCommunicationMatrix = (selectedPath: string, client: Client): void => {
    const communicationMatrixFile = findFilesByPath(selectedPath, 'cluster_communication_matrix.json');
    if (communicationMatrixFile.length === 0) {
        logger.info('communication matrix file is not found.');
        return;
    }
    parseCommunicationMatrixFile(communicationMatrixFile, client);
};

export const importKernelDetail = (parentPath: string, rankId: string): void => {
    const kernelDetailFileArr = findFilesByPath(parentPath, 'kernel_details.csv');
    if (kernelDetailFileArr.length === 0) {
        logger.info('kernel_details.csv file is not found.');
        return;
    }
    parseKernelDetail(rankId, kernelDetailFileArr);
};

const importStepStatistics = (selectedPath: string): void => {
    const stepStatisticsFiles = findFilesByPath(selectedPath, 'cluster_step_trace_time.csv');
    if (stepStatisticsFiles.length === 0) {
        logger.info('cluster_step_trace_time.csv file is not found.');
        return;
    }
    parseStepStatisticsFile(stepStatisticsFiles);
};
