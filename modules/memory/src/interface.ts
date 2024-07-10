/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { NotificationHandler } from './connection/defs';
import {
    parseMemoryCompletedHandler,
    removeRemoteHandler,
    setTheme,
    updateSessionHandler,
    deleteRankHandler,
    allSuccessHandler,
    switchLanguageHandler,
} from './connection/handler';

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    'parse/memoryCompleted': parseMemoryCompletedHandler,
    allPagesSuccess: allSuccessHandler,
    'remote/remove': removeRemoteHandler,
    'remote/reset': removeRemoteHandler,
    'module.reset': removeRemoteHandler,
    updateSession: updateSessionHandler,
    setTheme,
    deleteRank: deleteRankHandler,
    switchLanguage: switchLanguageHandler,
};
