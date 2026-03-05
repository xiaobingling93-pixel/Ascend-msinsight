/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import { store } from '@/store';
import { runInAction } from 'mobx';
import type { NotificationHandler } from './defs';
import i18n from '@insight/lib/i18n';
import { workerDestroy } from '@/leaksWorker/blockWorker/worker';
import { errorCenter, ErrorCode, WsError } from '@insight/lib';
import { workerDestroy as stateWorkerDestroy } from '@/leaksWorker/stateWorker/worker';

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const updateSessionHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session || typeof session !== 'object' || typeof data !== 'object') {
            return;
        }
        const dataKeys = Object.keys(data);
        const sessionKeys = Object.keys(session);
        dataKeys.forEach((key: string) => {
            if (sessionKeys.includes(key)) {
                (session as unknown as Record<string, unknown>)[key] = data[key];
            }
        });
    });
};

export const switchLanguageHandler: NotificationHandler = (data): void => {
    const session = store.sessionStore.activeSession;
    const lang = data.lang as 'zhCN' | 'enUS';
    if (session) {
        runInAction(() => {
            session.language = lang;
        });
    }
    i18n.changeLanguage(lang);
};

export const parseCompletedHandler = (): void => {
    const session = store.sessionStore.activeSession;
    workerDestroy();
    if (session) {
        runInAction(() => {
            session.renderId = ++session.renderId % 1000;
            session.tritonParsed = true;
        });
    }
};
export const removeRemoteHandler: NotificationHandler = (): void => {
    workerDestroy();
    stateWorkerDestroy();
};

export const parseFailHandler: NotificationHandler = (data): void => {
    errorCenter.handleError(new WsError(ErrorCode.PARSE_FAIL, data.error as string));
};
