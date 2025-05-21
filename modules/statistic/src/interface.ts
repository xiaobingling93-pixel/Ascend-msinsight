/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import type { NotificationHandler } from './connection/defs';
import {
    removeRemoteHandler,
    setTheme,
    updateSessionHandler,
    deleteRankHandler,
    allSuccessHandler,
    switchLanguageHandler,
    switchDirectoryHandler,
    parseStatisticCompletedHandler,
} from './connection/handler';

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    'parse/statisticCompleted': parseStatisticCompletedHandler,
    allPagesSuccess: allSuccessHandler,
    'remote/remove': removeRemoteHandler,
    'remote/reset': removeRemoteHandler,
    'module.reset': removeRemoteHandler,
    updateSession: updateSessionHandler,
    setTheme,
    deleteRank: deleteRankHandler,
    switchLanguage: switchLanguageHandler,
    switchDirectory: switchDirectoryHandler,
};
