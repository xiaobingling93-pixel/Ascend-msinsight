import * as fs from 'fs';
import * as path from 'path';
import { parseCardID } from '../utils/common_util';
import { parse } from '../parse/main';
import { Client } from '../types';
import { queryUnitsMetadata } from '../query/unitMetadataHandler';
import { exec } from 'child_process';
import * as os from 'os';
import { promisify } from 'util';
import { getLoggerByName } from '../logger/loggger_configure';
import { parseKernelDetail } from '../parse/kernel_detail.parser';
import { parseCommunicationFile, parseStepStatisticsFile, saveClusterBaseInfo } from '../parse/communication_parser';
import { CLUSTER_DATABASE } from '../database/tableManager';

const logger = getLoggerByName('import', 'info');
const execute = promisify(exec);

function findJsonFiles(dir: string, matchedFilePaths: string[], depth: number, fileFormatter: RegExp): void {
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

export async function selectFolder(): Promise<string | null> {
    if (os.platform() === 'win32') {
        return await selectFolderWindows();
    } else if (os.platform() === 'linux') {
        return await selectFolderLinux();
    }
    return null;
}

function findFilesByPath(path: string | null, fileNameFormatter: string): string[] {
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

export function splitRankFile(matchedFilePaths: string[]): Map<string, string[]> {
    const resultMap = new Map<string, string[]>();
    matchedFilePaths.forEach(filePath => {
        const tempRankId = parseCardID(filePath);
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

export const importHandler = async (req: { path: string | null }, client: Client): Promise<Record<string, unknown>> => {
    let selectedFolder = req.path;
    selectedFolder = req.path === 'browser' ? await selectFolder() : selectedFolder;
    let scene = 'train';
    let timeLineJsonFileArr = findFilesByPath(selectedFolder, 'trace_view.json');
    // 没有找到去查找msprof文件
    if (timeLineJsonFileArr.length === 0) {
        logger.info('trace_view.json file is not found, then go to find infer msprof file.');
        scene = 'infer';
        timeLineJsonFileArr = findFilesByPath(selectedFolder, 'msprof_[0-9]{1,}_[0-9]{1,}_[0-9]{1,}.json');
    }
    const timeLineJsonFileMap = splitRankFile(timeLineJsonFileArr);
    const importedRankIdSet = client.shadowSession.importedRankIdSet;
    const extremumTimestamp = client.shadowSession.extremumTimestamp;
    const result: {cards: CardInfo[]; steps: string[]; scene: string} = { cards: [], steps: [], scene };
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
        // import kernel detail data
        importedRankIdSet.add(rankId);
    }
    // 多卡场景才处理
    if (selectedFolder !== null && scene === 'train' && result.cards.length > 1) {
        logger.info('generate train cluster tables');
        await CLUSTER_DATABASE.createClusterTable();
        // import communication data
        importCommunication(selectedFolder, client);
        // write base info table
        saveClusterBaseInfo(selectedFolder);
        // import step statistics data
        importStepStatistics(selectedFolder);
    }
    return result;
};

export const importCommunication = (selectedPath: string, client: Client): void => {
    const communicationFileArr = findFilesByPath(selectedPath, 'communication.json');
    if (communicationFileArr.length === 0) {
        logger.info('communication file is not found.');
        return;
    }
    parseCommunicationFile(communicationFileArr);
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
    const stepStatisticsFiles = findFilesByPath(selectedPath, 'step_statistics.csv');
    if (stepStatisticsFiles.length === 0) {
        logger.info('step_statistics.csv file is not found.');
        return;
    }
    parseStepStatisticsFile(stepStatisticsFiles);
};
