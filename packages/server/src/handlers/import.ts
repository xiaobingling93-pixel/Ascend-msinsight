import { exec } from 'child_process';
import { promisify } from 'util';
import * as fs from 'fs';
import * as path from 'path';
import { parseCardID } from '../utils/common_util';
import { parse } from '../parse/main';
import { Client } from '../types';
import { queryUnitsMetadata } from '../query/unitMetadataHandler';
import { ExtremumTimestamp } from '../query/data';

const execAsync = promisify(exec);

const script = 'Add-Type -AssemblyName System.Windows.Forms; $dialog = New-Object System.Windows.Forms.FolderBrowserDialog;$result = $dialog.ShowDialog(); if ($result -eq “OK”) { $dialog.SelectedPath }';

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

async function findTraceViewJson(): Promise<string[]> {
    const traceViewJsonPaths: string[] = [];
    try {
        const { stdout } = await execAsync(`PowerShell -Command "${script}`);
        const folderPath = stdout.trim();
        if (!folderPath) {
            return traceViewJsonPaths;
        }
        findJsonFiles(folderPath, traceViewJsonPaths, 0);
    } catch (error) {
        console.error(error);
    }
    return traceViewJsonPaths;
}

export const importedRankIdSet = new Set<number>();

type CardInfo = {
    cardName: string;
    rankId: number;
};

export const extremumTimestamp: ExtremumTimestamp = {
    minTimestamp: Number.MAX_VALUE,
    maxTimestamp: Number.MIN_VALUE,
};

export const importHandler = async (req: any, client: Client): Promise<Record<string, unknown>> => {
    const traceViewJsonPaths = await findTraceViewJson();
    const result: CardInfo[] = [];
    for (const traceViewJsonPath of traceViewJsonPaths) {
        const rankId = parseCardID(traceViewJsonPath);
        if (importedRankIdSet.has(rankId)) {
            continue;
        };
        result.push({ cardName: rankId.toString(), rankId });
        parse(traceViewJsonPath, rankId, (ranId, err) => {
            if (err) {
                // this to send parse file error message
                console.log(err);
            }
            // this to send parse file success message
            queryUnitsMetadata(rankId).then((queryResult) => {
                extremumTimestamp.minTimestamp = Math.min(extremumTimestamp.minTimestamp, queryResult.extremumTimestamp.minTimestamp);
                client?.notify('parse/success', { unit: queryResult.insightMetaData, timeStamp: queryResult.extremumTimestamp });
            });
            console.log('send notify rankId parse end. ', rankId);
        });
        importedRankIdSet.add(rankId);
    }
    return { result };
};
