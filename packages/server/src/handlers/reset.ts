import { tableMap } from '../database/tableManager';
import * as fs from 'fs';
import { Client } from '../types';
import { getLoggerByName } from '../logger/loggger_configure';

const logger = getLoggerByName('reset', 'info');
export const resetHandler = async (req: any, client: Client): Promise<Record<string, unknown>> => {
    client.shadowSession.importedRankIdSet.clear();
    client.shadowSession.extremumTimestamp = { minTimestamp: Number.MAX_VALUE, maxTimestamp: Number.MIN_VALUE };
    await terminateParse();
    tableMap.forEach(async table => {
        await table.close();
        fs.unlink(table.dbPath, (err) => {
            if (err) {
                logger.error(err);
                return;
            }
            logger.info('File deleted successfully');
        });
    });
    tableMap.clear();
    return { };
};
