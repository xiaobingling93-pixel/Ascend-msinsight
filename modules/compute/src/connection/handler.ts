/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { store } from '../store';
import { runInAction } from 'mobx';
import { type NotificationHandler } from './defs';

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const importRemoteHandler = (): void => {
    reset();
};

export const updateSessionHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        const dataKeys = Object.keys(data);
        const sessionKeys = Object.keys(session);
        const updateData = {};
        dataKeys.forEach((key: string) => {
            if (sessionKeys.includes(key)) {
                Object.assign(updateData, { [key]: data[key] });
            }
        });
        if (Object.keys(updateData).length > 0) {
            Object.assign(session, {
                ...updateData,
                parseStatus: true,
                updateId: ++session.updateId % 100,
            });
        }
    });
};

export const resetHandler: NotificationHandler = (data): void => {
    reset();
};
const reset = (): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        Object.assign(session, {
            coreList: [],
            sourceList: [],
            parseStatus: false,
        });
    });
};
