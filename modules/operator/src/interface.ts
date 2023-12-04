/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { NotificationHandler } from './connection/defs';
import { setTheme, updateSessionHandler, parseSuccessHandler, resetHandler, deleteRankHandler } from './connection/handler';

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    setTheme,
    updateSession: updateSessionHandler,
    'parse/operatorCompleted': parseSuccessHandler,
    'remote/remove': resetHandler,
    'remote/reset': resetHandler,
    deleteRank: deleteRankHandler,
};
