import { getLogger } from 'log4js';
import { Client } from '../types';
import { InsightError } from '../utils/error';
import { isEmpty } from 'lodash';

export const loggerHandler = async (req: { logName: string; level: string; message: string}, client: Client): Promise<Record<string, unknown>> => {
    if (isEmpty(req.logName)) {
        throw new InsightError(1, 'logName is empty');
    }
    const logger = getLogger(req.logName);
    logger.level = req.level;
    logger.log(logger.level, req.message);
    return { code: 0 };
};
