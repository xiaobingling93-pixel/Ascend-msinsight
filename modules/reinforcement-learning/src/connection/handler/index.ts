/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { NotificationHandler } from '@/connection/defs';
import { setThemeHandler, switchLanguageHandler } from '@/connection/handler/settingsHandler';
import { updateSessionHandler } from '@/connection/handler/updateSessionHandler';
import { removeHandler, resetHandler } from '@/connection/handler/removeHandler';

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler<any>> = {
    setTheme: setThemeHandler,
    switchLanguage: switchLanguageHandler,
    updateSession: updateSessionHandler,
    'remote/remove': removeHandler,
    'remote/reset': resetHandler,
};
