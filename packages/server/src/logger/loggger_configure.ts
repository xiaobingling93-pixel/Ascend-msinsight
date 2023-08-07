import { configure, Logger, getLogger } from 'log4js';
import { join } from 'path';
import { CACHE_PATH } from '../common/common';

configure({
    appenders: { server: { type: 'file', filename: join(CACHE_PATH, 'server.log'), maxLogSize: 10485760 } },
    categories: { default: { appenders: ['server'], level: 'info' } },
});

export function getLoggerByName(name: string, level: string): Logger {
    const logger = getLogger(name);
    logger.level = level;
    return logger;
}
