import { tableMap } from '../database/tableManager';
import * as fs from 'fs';
import { Client } from '../types';
import { terminateParse } from '../parse/main';
import { getLoggerByName } from '../logger/loggger_configure';

const logger = getLoggerByName('reset', 'info');
export let tableMapClearFlag = false;
export const resetHandler = async (req: any, client: Client): Promise<Record<string, unknown>> => {
    tableMapClearFlag = true;
    client.shadowSession.importedRankIdSet.clear();
    client.shadowSession.extremumTimestamp = { minTimestamp: Number.MAX_VALUE, maxTimestamp: -Number.MAX_VALUE };
    await terminateParse();
    tableMap.forEach(async table => {
        await table.close();
        const path = `${table.dbPath}.tmp`;
        fs.renameSync(table.dbPath, path);
        fs.unlink(path, (err) => {
            if (err) {
                logger.error(err);
                return;
            }
            logger.info('File deleted successfully');
        });
    });
    tableMap.clear();
    tableMapClearFlag = false;
    logger.info('reset completed.');
    return { };
};

export async function waitResetComplete(): Promise<void> {
    logger.info(`wait reset completed. flag:${tableMapClearFlag}`);
    return new Promise(resolve => {
        if (!tableMapClearFlag) {
            resolve();
        }
        const timer = setInterval(() => {
            if (!tableMapClearFlag) {
                clearInterval(timer);
                resolve();
            }
        });
    });
}
