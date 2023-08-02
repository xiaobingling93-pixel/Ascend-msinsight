import * as fs from 'fs';
import * as path from 'path';
import { parseCardID } from '../utils/common_util';
import { parse } from '../parse/main';
import { Client } from '../types';
import { queryUnitsMetadata } from '../query/unitMetadataHandler';
import { exec } from 'child_process';
import * as os from 'os';
import { promisify } from 'util';

const execute = promisify(exec);
function findJsonFiles(dir: string, traceViewJsonPaths: string[], depth: number): void {
    if (depth > 5) return; // 控制递归深度
    const files = fs.readdirSync(dir);
    for (const file of files) {
        const filePath = path.join(dir, file);
        const stats = fs.statSync(filePath);
        if (stats.isDirectory()) {
            findJsonFiles(filePath, traceViewJsonPaths, depth + 1);
        } else if (file === 'trace_view.json') {
            traceViewJsonPaths.push(filePath);
        }
    }
}

async function selectFolderWindows(): Promise<string> {
    const script = '[Console]::OutputEncoding = [System.Text.Encoding]::UTF8; Add-Type -AssemblyName System.Windows.Forms; $dialog = New-Object System.Windows.Forms.FolderBrowserDialog;$result = $dialog.ShowDialog(); if ($result -eq “OK”) { $dialog.SelectedPath }';
    try {
        const { stdout } = await execute(`PowerShell -Command "${script}"`);
        const folderPath = stdout.trim();
        return folderPath;
    } catch (error) {
        console.error(error);
        return '';
    }
}

async function selectFolderLinux(): Promise<string> {
    try {
        const { stdout } = await execute('zenity --file-selection --directory');
        const folderPath = stdout.trim();
        return folderPath;
    } catch (error) {
        console.error(error);
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

async function findTraceViewJson(path: string): Promise<string[]> {
    const traceViewJsonPaths: string[] = [];
    try {
        if (path === 'browser') {
            await selectFolder().then(folderPath => {
                if (folderPath === null) {
                    return traceViewJsonPaths;
                }
                findJsonFiles(folderPath, traceViewJsonPaths, 0);
            });
        } else {
            findJsonFiles(path, traceViewJsonPaths, 0);
        }
    } catch (error) {
        console.error(error);
    }
    return traceViewJsonPaths;
}

type CardInfo = {
    cardName: string;
    rankId: string;
};

export const importHandler = async (req: { path: string }, client: Client): Promise<Record<string, unknown>> => {
    const traceViewJsonPaths = await findTraceViewJson(req.path);
    const importedRankIdSet = client.shadowSession.importedRankIdSet;
    const extremumTimestamp = client.shadowSession.extremumTimestamp;
    const result: CardInfo[] = [];
    for (const traceViewJsonPath of traceViewJsonPaths) {
        const rankId = parseCardID(traceViewJsonPath);
        if (importedRankIdSet.has(rankId)) {
            continue;
        };
        result.push({ cardName: rankId.toString(), rankId });
        parse(traceViewJsonPath, rankId, (rankId, err) => {
            if (err) {
                // this to send parse file error message
                console.log(err);
            }
            // this to send parse file success message
            queryUnitsMetadata(rankId).then((queryResult) => {
                let startTimeUpdated = false;
                if (extremumTimestamp.minTimestamp > queryResult.extremumTimestamp.minTimestamp) {
                    extremumTimestamp.maxTimestamp = extremumTimestamp.maxTimestamp + extremumTimestamp.minTimestamp - queryResult.extremumTimestamp.minTimestamp;
                    extremumTimestamp.minTimestamp = queryResult.extremumTimestamp.minTimestamp;
                    startTimeUpdated = true;
                };
                extremumTimestamp.maxTimestamp = Math.max(queryResult.extremumTimestamp.maxTimestamp - extremumTimestamp.minTimestamp, extremumTimestamp.maxTimestamp);
                client?.notify('parse/success', { unit: queryResult.insightMetaData, startTimeUpdated, maxTimeStamp: extremumTimestamp.maxTimestamp });
            });
            console.log('send notify rankId parse end. ', rankId);
        });
        importedRankIdSet.add(rankId);
    }
    return { result };
};
