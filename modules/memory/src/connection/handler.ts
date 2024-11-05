/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { store } from '../store';
import { runInAction } from 'mobx';
import type { NotificationHandler } from './defs';
import type { RankInfo } from '../entity/memory';
import i18n from 'ascend-i18n';
import { customConsole as console } from 'ascend-utils';
export const parseMemoryCompletedHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            const memoryResult = data.memoryResult as RankInfo[];
            const isCluster = data.isCluster as boolean;
            if (!isCluster && !session.isCluster) {
                memoryResult.forEach((item) => {
                    if (!session.memoryRankIds.includes(item.rankId) && (item.hasMemory as boolean)) {
                        session.memoryRankIds.push(item.rankId);
                    }
                });
            } else {
                session.memoryRankIds = [];
                memoryResult.forEach((item) => {
                    if (item.hasMemory) {
                        session.memoryRankIds.push(item.rankId);
                    }
                });
            }
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
            session.memoryRankIds = [];
            session.isCluster = false;
            session.curRankIdsCount = 0;
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
            const usableKeys: string[] = ['isCluster', 'unitcount'];
            dataKeys.forEach((key: any) => {
                if (key === 'memoryRankIds') {
                    const memoryRankIds = data.memoryRankIds as string[];
                    session.memoryRankIds = [...new Set([...session.memoryRankIds, ...memoryRankIds])];
                    return;
                }
                if (usableKeys.includes(key)) {
                    (session as any)[key] = data[key];
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
            if (data.isAllPageParsed as boolean && session.shouldRefresh) {
                session.isClusterMemoryCompletedSwitch = !session.isClusterMemoryCompletedSwitch;
            }
            session.shouldRefresh = true;
        });
    } catch (error) {
        console.error(error);
    }
};

export const deleteRankHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            const deleteIds: string[] = data.rankId as string[];
            if (deleteIds.length > 0) {
                session.memoryRankIds = session.memoryRankIds.filter((item: string) => !deleteIds?.includes(item));
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
