/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
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
import { runInAction } from 'mobx';
import i18n from '@insight/lib/i18n';
import { customConsole as console } from '@insight/lib/utils';
import { store } from '../store';
import type { NotificationHandler } from './defs';
export const parseStatisticCompletedHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            const rankIds = data.rankIds as string[];
            rankIds.forEach((item) => {
                if (!session.iERankIds.includes(item)) {
                    session.iERankIds.push(item);
                }
            });
        });
    } catch (err) {
        console.error(err);
    }
};

export const locateGroup: NotificationHandler = (data): void => {
    try {
        const { memoryStore } = store;
        const curveSession = memoryStore.activeSession;
        runInAction(() => {
            if (!curveSession) {
                return;
            }
            curveSession.groupId = data.group as string;
            const exists = curveSession.groupCondition.some(item => item.label === curveSession.groupId);
            if (!exists) {
                curveSession.groupCondition.push({ label: curveSession.groupId, value: curveSession.groupId });
            }
            curveSession.rankIdCondition.value = data.fileId as string;
        });
    } catch (err) {
        console.error(err);
    }
};

export const removeRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore, memoryStore } = store;
        const session = sessionStore.activeSession;
        const memorySession = memoryStore.activeSession;
        runInAction(() => {
            if (!session || !memorySession) {
                return;
            }
            session.iERankIds = [];
            session.isCluster = false;
            session.compareRank.rankId = '';
            memorySession.rankIdCondition = { options: [], value: '' };
        });
    } catch (error) {
        console.error(error);
    }
};

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const updateSessionHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            const dataKeys = Object.keys(data);
            dataKeys.forEach((key: any) => {
                if (key === 'iERankIds') {
                    const iERankIds = data.iERankIds as string[];
                    session.iERankIds = [...new Set([...session.iERankIds, ...iERankIds])];
                }
            });
        });
    } catch (error) {
        console.error(error);
    }
};

export const allSuccessHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            session.shouldRefresh = true;
        });
    } catch (error) {
        console.error(error);
    }
};

export const deleteCardHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            const deleteIds: Set<string> = new Set((data.info as Array<{ cardId: string; dbPath: string }>).map(({ cardId }) => cardId));
            if (deleteIds.size > 0) {
                session.iERankIds = session.iERankIds.filter((item: string) => !deleteIds.has(item));
            }
        });
    } catch (error) {
        console.error(error);
    }
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

export const switchDirectoryHandler: NotificationHandler = (data): void => {
    const session = store.sessionStore.activeSession;
    if (session) {
        runInAction(() => {
            session.shouldRefresh = false;
            session.compareRank = { rankId: data.rankId as string, isCompare: data.isCompare as boolean };
        });
    }
};
