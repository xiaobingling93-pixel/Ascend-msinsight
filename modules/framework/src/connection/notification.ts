/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import {
    deleteRankHandler,
    getParseStatusHandler,
    getThemeHandler,
    switchModuleHandler,
    updateSessionHandler,
    getLanguageHandler,
} from './notificationHandler';
import { sendWakeupPlugin } from '@/connection/sendNotification';

export interface NotificationMessage {
    data: {
        body: {
            [p: string]: unknown;
            from?: string;
            broadcast?: boolean;
            switchTo?: string;
            toModuleEvent?: string;
        };
        event: string;
        module: string;
        from?: number;
        modules?: string[];
        port?: number;
    };
    isTrusted?: boolean;
}

type ListenerCallback = (res: NotificationMessage) => void;

export const listenerMap: Record<string, ListenerCallback> = {
    updateSession: updateSessionHandler,
    getParseStatus: getParseStatusHandler,
    getTheme: getThemeHandler,
    deleteRank: deleteRankHandler,
    switchModule: switchModuleHandler,
    getLanguage: getLanguageHandler,
    pluginMounted: sendWakeupPlugin,
};
