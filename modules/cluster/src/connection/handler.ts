/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { store } from '../store';
import { runInAction } from 'mobx';
import type { NotificationHandler } from './defs';
import { updateData, AnalysisType } from '../components/communication/Filter';
import type { ConditionDataType } from '../components/communication/Filter';
import i18n from 'ascend-i18n';
import type { communicatorContainerData } from '../components/communicatorContainer/ContainerUtils';
import { customConsole as console } from 'ascend-utils';

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
        session.clusterCompleted = true;
        session.renderId = ++session.renderId % 1000;
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
        session.allRankIds = [];
        session.communicatorData = { partitionModes: [], defaultPPSize: 0 };
        session.activeCommunicator = undefined;
        session.indicatorList = [];
        session.performanceData = [];
        session.communicationDomains = [];
        session.stepList = [];
    });
};

export const locateHCCL: NotificationHandler = (data): void => {
    const { iterationId, operatorName, stage } = data as unknown as ConditionDataType;
    updateData({
        iterationId,
        stage,
        operatorName: iterationId && stage && operatorName,
        type: AnalysisType.COMMUNICATION_DURATION_ANALYSIS,
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

export const updateCommunicatorDataHandler: NotificationHandler = (data): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.communicatorData = data as unknown as communicatorContainerData;
    });
};
