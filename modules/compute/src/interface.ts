/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { type NotificationHandler } from './connection/defs';
import {
    setTheme,
    importRemoteHandler,
    updateSessionHandler,
    resetHandler,
    switchLanguageHandler,
    switchDirectoryHandler,
    showCacheInstructionsHandler,
} from './connection/handler';

const IMPORT = 'remote/import';
const REMOVE = 'remote/remove';
const RESET = 'remote/reset';
export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    setTheme,
    [IMPORT]: importRemoteHandler,
    updateSession: updateSessionHandler,
    [REMOVE]: resetHandler,
    [RESET]: resetHandler,
    switchLanguage: switchLanguageHandler,
    switchDirectory: switchDirectoryHandler,
    showCacheInstructions: showCacheInstructionsHandler,
};
