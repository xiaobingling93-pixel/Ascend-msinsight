/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { store } from '../store';
import { runInAction } from 'mobx';
import { type NotificationHandler } from './defs';

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
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
        dataKeys.forEach((key: string) => {
            if (sessionKeys.includes(key)) {
                (session as unknown as Record<string, unknown>)[key] = data[key];
            }
        });
        session.renderStatus = !session.renderStatus;
    });
};

export const resetHandler: NotificationHandler = (data): void => {
    resetStatus();
};
const resetStatus = (): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.renderStatus = !session.renderStatus;
    });
};
