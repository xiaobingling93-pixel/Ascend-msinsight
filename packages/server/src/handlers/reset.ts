import { tableMap } from '../database/tableManager';
import * as fs from 'fs';
import { Client } from '../types';
import { terminateParse } from '../parse/main';

export const resetHandler = async (req: any, client: Client): Promise<Record<string, unknown>> => {
    client.shadowSession.importedRankIdSet.clear();
    client.shadowSession.extremumTimestamp = { minTimestamp: Number.MAX_VALUE, maxTimestamp: Number.MIN_VALUE };
    await terminateParse();
    tableMap.forEach(async table => {
        await table.close();
        fs.unlink(table.dbPath, (err) => {
            if (err) {
                console.error(err);
                return;
            }
            console.log('File deleted successfully');
        });
    });
    tableMap.clear();
    return { };
};
