import { exec } from 'child_process';
import { promisify } from 'util';
import * as fs from 'fs';
import * as path from 'path';
import { parseCardID } from './parse/metadata';
import { parse } from './parse/main';

const execAsync = promisify(exec);

type FolderInfo = {
    cardName: string;
    cardPath: string;
};

const folderSet = new Set<string>();
const script = 'Add-Type -AssemblyName System.Windows.Forms; $dialog = New-Object System.Windows.Forms.FolderBrowserDialog; $result = $dialog.ShowDialog(); if ($result -eq “OK”) { $dialog.SelectedPath }';
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
                if (folderSet.has(subfolderPath)) {
                    continue;
                }
                folderSet.add(subfolderPath);
                result.push({
                    cardName: subfolder,
                    cardPath: subfolderPath,
                });
            }
        } else if (path.basename(path.dirname(folderPath)) === deviceFolder) {
            if (folderSet.has(folderPath)) {
                return result;
            }
            folderSet.add(folderPath);
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

export const importHandler = async (): Promise<Record<string, unknown>> => {
    const folders = await selectFolders();
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
            parse(filePath, rankId, (err, rankId) => {
                if (err) {
                    // this to send parse file error message
                    console.log(err);
                }
                // this to send parse file success message
                console.log('send notify rankId parse end. ', rankId);
            });
        }
    }
    return {
        result: [folders],
    };
};
