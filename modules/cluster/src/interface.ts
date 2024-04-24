/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { NotificationHandler } from './connection/defs';
import { removeRemoteHandler, parseClusterSuccessHandler, setTheme, moduleMessageHandler, updateSessionHandler, locateHCCL } from './connection/handler';

type InsightInterface<Request extends Record<string, unknown>, Response extends Record<string, unknown>> = {
    request: Request;
    response: Response;
};

export type InterfaceDefs = {
    'chart/cpu': InsightInterface<{ chartId: number }, { data: Array<{ ts: number; value: number }>}>;
};

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    'remote/remove': removeRemoteHandler,
    'remote/reset': removeRemoteHandler,
    'module.reset': removeRemoteHandler,
    setTheme,
    'parse/clusterCompleted': parseClusterSuccessHandler,
    moduleMessage: moduleMessageHandler,
    updateSession: updateSessionHandler,
    locateHCCL,
};
