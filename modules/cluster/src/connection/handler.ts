/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { store } from '../store';
import { runInAction } from 'mobx';
import { setUnitPhaseByCardId } from '../entity/insight';
import { NotificationHandler } from './defs';

export const parseSuccessHandler: NotificationHandler = (data): void => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            session.startRecordTime = 0;
        });
    } catch (error) {
        console.error(error);
    }
};

export const parseFailHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        setUnitPhaseByCardId((data as any).rankId, session, 'error');
    });
};

export const removeRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        resetStatus();
    } catch (error) {
        console.error(error);
    }
};

export const importRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        resetStatus();
    } catch (error) {
        console.error(error);
    }
};

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const parseClusterSuccessHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.clusterStatus = true;
        window.dataSource = data.dataSource as DataSource;
    });
};

export const moduleMessageHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.clusterStatus = true;
    });
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
        dataKeys.forEach((key: any) => {
            if (sessionKeys.includes(key)) {
                (session as any)[key] = data[key];
            }
        });
        if (data.isReset === true) {
            resetStatus();
        }
        session.renderId = session.renderId++ % 1000;
    });
};

const resetStatus = (): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.clusterCompleted = false;
        session.parseCompleted = false;
        session.unitcount = 0;
    });
};
