/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import { type NotificationHandler } from './connection/defs';
import { setTheme, updateSessionHandler, switchLanguageHandler, parseCompletedHandler, removeRemoteHandler } from './connection/handler';

const PARSECOMPLETED = 'parse/leaksMemoryCompleted';
export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    setTheme,
    updateSession: updateSessionHandler,
    switchLanguage: switchLanguageHandler,
    [PARSECOMPLETED]: parseCompletedHandler,
    'remote/remove': removeRemoteHandler,
};
