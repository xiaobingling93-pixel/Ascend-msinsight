/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { type NotificationHandler } from './connection/defs';
import { setTheme, updateSessionHandler, resetHandler } from './connection/handler';

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    setTheme,
    updateSession: updateSessionHandler,
    'remote/remove': resetHandler,
    'remote/reset': resetHandler,
};
