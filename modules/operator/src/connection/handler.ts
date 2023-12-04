/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { store } from '../store';
import { runInAction } from 'mobx';
import { NotificationHandler } from './defs';

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const updateSessionHandler: NotificationHandler = (data): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        const dataKeys = Object.keys(data);
        const sessionKeys = Object.keys(session);
        dataKeys.forEach((key: any) => {
            if (sessionKeys.includes(key)) {
                (session as any)[key] = data[key];
            }
        });
        if (data.isReset === true) {
            resetHandler({});
        }
        session.renderId++;
    });
};

export const parseSuccessHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        const ids = [ ...session.allRankIds, String(data.rankId) ].sort((a, b) => Number(a) - Number(b));
        session.allRankIds = ids;
    });
};

export const resetHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.allRankIds = [];
    });
};
export const deleteRankHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        const deleteIds: string[] = data.rankId as string[];
        if (deleteIds.length > 0) {
            const rankIds = session.allRankIds.filter((item: string) => !deleteIds?.includes(item));
            session.allRankIds = rankIds;
        }
    });
};
