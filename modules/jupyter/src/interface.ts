/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { type NotificationHandler } from './connection/defs';
import { setTheme, updateSessionHandler, resetHandler } from './connection/handler';

const REMOVE = 'remote/remove';
const RESET = 'remote/reset';
export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    setTheme,
    updateSession: updateSessionHandler,
    [REMOVE]: resetHandler,
    [RESET]: resetHandler,
};
