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

type FolderInfo = {
    cardName: string;
    cardPath: string;
};

const script = 'Add-Type -AssemblyName System.Windows.Forms; $dialog = New-Object System.Windows.Forms.FolderBrowserDialog;$result = $dialog.ShowDialog(); if ($result -eq “OK”) { $dialog.SelectedPath }';

async function selectFolders(): Promise<FolderInfo[]> {
    const result: FolderInfo[] = [];
    try {
        const { stdout } = await execAsync(`PowerShell -Command "${script}`);
        const folderPath = stdout.trim();
        if (!folderPath) {
            return result;
        }
        const deviceFolder = folderPath.match(/\b\d+\.\d+\.\d+\.\d+\b/)?.[0];
        if (deviceFolder === undefined) {
            return result;
        }
        if (path.basename(folderPath) === deviceFolder) {
            const subfolders = fs.readdirSync(folderPath, { withFileTypes: true })
                .filter(dirent => dirent.isDirectory())
                .map(dirent => dirent.name);
            for (const subfolder of subfolders) {
                const subfolderPath = path.join(folderPath, subfolder);
                result.push({
                    cardName: subfolder,
                    cardPath: subfolderPath,
                });
            }
        } else if (path.basename(path.dirname(folderPath)) === deviceFolder) {
            const cardName = path.basename(folderPath);
            result.push({
                cardName,
                cardPath: folderPath,
            });
        }
    } catch (error) {
        console.error(error);
    }
    return result;
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
    const folders = await selectFolders();
    const result: CardInfo[] = [];
    for (const folder of folders) {
        const files = fs.readdirSync(folder.cardPath, { withFileTypes: true })
            .filter(dirent => dirent.isFile())
            .map(dirent => dirent.name);
        for (const file of files) {
            if (path.extname(file) !== '.json') {
                continue;
            }
            const filePath = path.join(folder.cardPath, file);
            const rankId = parseCardID(filePath);
            if (importedRankIdSet.has(rankId)) {
                continue;
            }
            importedRankIdSet.add(rankId);
            result.push({ cardName: folder.cardName, rankId });
            parse(filePath, rankId, (ranId, err) => {
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
        }
    }
    return { result };
};
