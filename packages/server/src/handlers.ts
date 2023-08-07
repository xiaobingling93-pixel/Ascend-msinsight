import { chartHandler, testHandler } from './handlers.mock';
import { threadTracesHandler } from './query/thread.trace.handler';
import { Handler } from './types';
import { flowDetailHandler, flowNameHandler, threadInfoHandler, threadsInfoHandler } from './query/thread.detail.handler';
import { importHandler } from './handlers/import';
import { unitMetadataHandler } from './query/unitMetadataHandler';
import { resetHandler } from './handlers/reset';
import { loggerHandler } from './logger/logger_handler';

export const HANDLER_MAP: Record<string, Handler> = {
    test: testHandler,
    'unit/chart': chartHandler,
    'import/action': importHandler,
    'reset/window': resetHandler,
    'unit/threadTraces': threadTracesHandler,
    'unit/queryUnitMetadata': unitMetadataHandler,
    'unit/threadDetail': threadInfoHandler,
    'unit/threads': threadsInfoHandler,
    'unit/flowName': flowNameHandler,
    'unit/flow': flowDetailHandler,
    'log/logger': loggerHandler,
};
