import { Parser, logger } from './parser';
import { parentPort } from 'worker_threads';
import { WorkMessageType, WorkStatus } from './threadPool';

export type ParseMessage = {
    rankId: string;
    filePath: string;
    dbPath: string;
    readPosition: number;
    readSize: number;
};

export type EndMessage = {
    rankId: string;
    count: number;
    ignoreCount: number;
};

export async function parseWorkerOnMessage(msg: { command: WorkMessageType; data: ParseMessage }): Promise<void> {
    switch (msg.command) {
        case WorkMessageType.PARSE: {
            logger.info(`[work] on message. read position: ${msg.data.readPosition}, read size: ${msg.data.readSize}`);
            const parser = new Parser(msg.data.filePath, msg.data.dbPath);
            await parser.parse(msg.data.readPosition, msg.data.readSize);
            await parser.parseEnd();
            const count = parser.getCount();
            const ignoreCount = parser.getIgnoreCount();
            logger.info(`[work] read ${count} data. ignore ${ignoreCount} data. time:`, new Date().getTime());
            parentPort?.postMessage({ status: WorkStatus.END, data: { rankId: msg.data.rankId, count, ignoreCount } });
            break;
        }
        case WorkMessageType.EXIT: {
            logger.info('[work] on message. Exit.');
            parentPort?.close();
            break;
        }
        default:
            logger.log('[work] Unknown message. ', msg);
    }
}
