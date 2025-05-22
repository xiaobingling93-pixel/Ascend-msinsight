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
import { ClusterInfo } from '../entity/session';
import { customConsole as console } from 'ascend-utils';

type LayerType = 'PROJECT' | 'CLUSTER' | 'HOST' | 'RANK' | 'COMPUTE' | 'IPYNB' | 'UNKNOWN';

export interface ClusterPageInfo {
    clusterList: ClusterInfo[];
    selectedClusterPath: string;
}

export interface TimelinePageInfo {
    unitCount: number;
}

interface ProjectCreateInfo extends Record<string, any> {
    selectedFileType: LayerType;
    selectedFilePath: string;
    selectedProjectName: string;
    pageInfo: {
        cluster: ClusterPageInfo;
        timeline: TimelinePageInfo;
    };
}

interface ProjectUpdateInfo extends ProjectCreateInfo {
    rankId: string;
    isCompare: boolean;
}

export const removeRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        resetStatus();
    } catch (error) {
        console.error(error);
    }
};

function updateClusterListAndSelectedAndTimelineStatus(pageInfo: ClusterPageInfo, selectedProjectName: string, unitCount: number): void {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) { return; }
        session.clusterList = pageInfo.clusterList;
        session.selectedClusterPath = pageInfo.selectedClusterPath;
        session.selectedProjectName = selectedProjectName;
        session.unitcount = unitCount;
    });
}

export const frameLoadedHandler: NotificationHandler<ProjectCreateInfo> = async (data: ProjectCreateInfo): Promise<void> => {
    try {
        resetStatus();
        updateClusterListAndSelectedAndTimelineStatus(data.pageInfo.cluster, data.selectedProjectName, data.pageInfo.timeline.unitCount);
    } catch (error) {
        console.error(error);
    }
};

export const switchDirectoryHandler: NotificationHandler<ProjectUpdateInfo> = (data: ProjectUpdateInfo): void => {
    try {
        const session = store.sessionStore.activeSession;
        if (data.selectedProjectName === session?.selectedProjectName) {
            resetStatus('Cluster');
        } else {
            resetStatus('Project');
        }
        updateClusterListAndSelectedAndTimelineStatus(data.pageInfo.cluster, data.selectedProjectName, data.pageInfo.timeline.unitCount);
    } catch (error) {
        console.error(error);
    }
};

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const updateClusterPageInfoHandler: NotificationHandler<ClusterPageInfo> = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.clusterList = data.clusterList;
        session.selectedClusterPath = data.selectedClusterPath;
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

const resetStatus = (type: 'Project' | 'Cluster' = 'Project'): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        if (type === 'Cluster') {
            session.resetForClusterChange();
        } else {
            session.resetForProjectChange();
        }
    });
};

export const locateCommunication: NotificationHandler = (data): void => {
    const { iterationId, operatorName, stage } = data as unknown as ConditionDataType;
    updateData({
        iterationId,
        stage,
        operatorName: iterationId && stage && operatorName,
        type: AnalysisType.COMMUNICATION_DURATION_ANALYSIS,
        baselineIterationId: '',
        pgName: '',
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

export const baselineToggleHandler: NotificationHandler = (data): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.isCompare = data.status as boolean ?? false;
        forceRender();
    });
};

function forceRender(): void {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.renderId = ++session.renderId % 1000;
    });
}

export const profilingExpertDataParsedHandler: NotificationHandler = (data, session): void => {
    runInAction(() => {
        if (!session) {
            return;
        }

        session.profilingExpertDataParsed = data.parseResult as boolean ?? false;
    });
};
