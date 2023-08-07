import { loggerHandler } from '../logger_handler';
import { InsightError } from '../../utils/error';
import { getLoggerByName } from '../loggger_configure';
import { levels } from 'log4js';

describe('logger test', () => {

    it('logger handler throw error when name is empty', async function () {
        const req: Record<string, any> = {
            logName: null,
            level: 'info',
            message: 'log test'
        };
        // @ts-ignore
        await expect(loggerHandler(req, null)).rejects
            .toEqual(new InsightError(1, 'logName is empty'));
        req.logName = undefined;
        // @ts-ignore
        await expect(loggerHandler(req, null)).rejects
            .toEqual(new InsightError(1, 'logName is empty'));
        req.logName = '';
        // @ts-ignore
        await expect(loggerHandler(req, null)).rejects
            .toEqual(new InsightError(1, 'logName is empty'));
    });

    it('logger handler log success', async function () {
        const req: Record<string, any> = {
            logName: 'test',
            level: 'info',
            message: 'log test'
        };
        // @ts-ignore
        await expect(loggerHandler(req, null)).resolves.toStrictEqual({code: 0})
        req.level = 'debug'
        // @ts-ignore
        await expect(loggerHandler(req, null)).resolves.toStrictEqual({code: 0})
        req.level = 'error'
        // @ts-ignore
        await expect(loggerHandler(req, null)).resolves.toStrictEqual({code: 0})
    });

    it('get logger is enabled when level is high than logger\'s level', async function () {
        let logger = getLoggerByName('test', 'info');
        expect(logger.isDebugEnabled()).toBe(false);
        expect(logger.isInfoEnabled()).toBe(true);
        expect(logger.level).toBe(levels.INFO);
        logger = getLoggerByName('test', 'test');
        expect(logger.isDebugEnabled()).toBe(false);
        expect(logger.isInfoEnabled()).toBe(true);
        expect(logger.level).toBe(levels.INFO);
    });
});
