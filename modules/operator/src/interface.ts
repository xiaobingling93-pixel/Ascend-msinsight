/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import type { NotificationHandler } from './connection/defs';
import {
    setTheme,
    updateSessionHandler,
    switchDirectoryHandler,
    parseSuccessHandler,
    resetHandler,
    deleteCardHandler,
    switchLanguageHandler,
    allSuccessHandler,
} from './connection/handler';

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    setTheme,
    updateSession: updateSessionHandler,
    switchDirectory: switchDirectoryHandler,
    'parse/operatorCompleted': parseSuccessHandler,
    'remote/remove': resetHandler,
    'remote/reset': resetHandler,
    'module.reset': resetHandler,
    deleteCard: deleteCardHandler,
    switchLanguage: switchLanguageHandler,
    allPagesSuccess: allSuccessHandler,
};
