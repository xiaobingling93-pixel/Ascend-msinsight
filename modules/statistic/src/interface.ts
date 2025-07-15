/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import type { NotificationHandler } from './connection/defs';
import {
    removeRemoteHandler,
    setTheme,
    updateSessionHandler,
    deleteCardHandler,
    allSuccessHandler,
    switchLanguageHandler,
    switchDirectoryHandler,
    parseStatisticCompletedHandler, locateGroup,
} from './connection/handler';

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    'parse/statisticCompleted': parseStatisticCompletedHandler,
    allPagesSuccess: allSuccessHandler,
    locateGroup,
    'remote/remove': removeRemoteHandler,
    'remote/reset': removeRemoteHandler,
    'module.reset': removeRemoteHandler,
    updateSession: updateSessionHandler,
    setTheme,
    deleteCard: deleteCardHandler,
    switchLanguage: switchLanguageHandler,
    switchDirectory: switchDirectoryHandler,
};
