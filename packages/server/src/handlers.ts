import { chartHandler, testHandler } from './handlers.mock';
import { threadTracesHandler } from './trhead.trace.handler';
import { Handler } from './types';
import { importHandler } from './import';
import { flowDetailHandler, flowNameHandler, threadInfoHandler, threadsInfoHandler } from './query/thread.detail.handler';
import { unitMetadataHandler } from './query/unitMetadataHandler';

export const HANDLER_MAP: Record<string, Handler> = {
    test: testHandler,
    'unit/chart': chartHandler,
    importCard: importHandler,
    'unit/threadTraces': threadTracesHandler,
    'unit/queryUnitMetadata': unitMetadataHandler,
    'unit/threadDetail': threadInfoHandler,
    'unit/threads': threadsInfoHandler,
    'unit/flowName': flowNameHandler,
    'unit/flow': flowDetailHandler,
};
