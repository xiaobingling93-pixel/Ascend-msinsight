/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { store } from '../store';
import { runInAction } from 'mobx';
import { NotificationHandler } from './defs';
import { updateData } from '../components/communicationAnalysis/Filter';
import type { ConditionDataType } from '../components/communicationAnalysis/Filter';

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
        session.clusterCompleted = true;
        session.renderId = ++session.renderId % 1000;
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
        session.renderId = ++session.renderId % 1000;
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
        session.durationFileCompleted = false;
    });
};

export const locateHCCL: NotificationHandler = (data): void => {
    const { iterationId, operatorName, stage } = data as unknown as ConditionDataType;
    updateData({
        iterationId,
        stage,
        operatorName: iterationId && stage && operatorName,
        rankIds: [],
        type: 'CommunicationDurationAnalysis',
    });
};
