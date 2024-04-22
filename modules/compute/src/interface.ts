/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { type NotificationHandler } from './connection/defs';
import { setTheme, importRemoteHandler, updateSessionHandler, resetHandler } from './connection/handler';

const IMPORT = 'remote/import';
export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    setTheme,
    [IMPORT]: importRemoteHandler,
    updateSession: updateSessionHandler,
    'remote/remove': resetHandler,
    'remote/reset': resetHandler,
};
