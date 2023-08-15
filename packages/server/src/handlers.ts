/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { chartHandler, testHandler } from './handlers.mock';
import { threadTracesHandler } from './query/thread.trace.handler';
import { Handler } from './types';
import { flowDetailHandler, flowNameHandler, threadInfoHandler, threadsInfoHandler } from './query/thread.detail.handler';
import { importHandler } from './handlers/import';
import { unitMetadataHandler } from './query/unitMetadataHandler';
import { resetHandler } from './handlers/reset';
import { summaryHandler, summaryStatisticHandler } from './query/summary.handler';
import { loggerHandler } from './logger/logger_handler';
import { communicationDetailInfoHandler, computeDetailInfoHandler } from './query/summary.detail.handler';
import {
    iterationsHandler,
    ranksHandler,
    operatorNamesHandler,
    durationListHandler,
    operatorDetailsHandler,
    bandwidthHandler,
    distributionHandler,
} from './query/communication.analysis.handler';

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
    'summary/queryComputeDetail': computeDetailInfoHandler,
    'summary/queryCommunicationDetail': communicationDetailInfoHandler,
    'communication/duration/iterations': iterationsHandler,
    'communication/duration/ranks': ranksHandler,
    'communication/duration/operatorNames': operatorNamesHandler,
    'communication/duration/list': durationListHandler,
    'communication/duration/operatorDetails': operatorDetailsHandler,
    'communication/duration/bandwidth': bandwidthHandler,
    'communication/duration/distribution': distributionHandler,
    'summary/queryTopData': summaryHandler,
    'summary/statistic': summaryStatisticHandler,
};
