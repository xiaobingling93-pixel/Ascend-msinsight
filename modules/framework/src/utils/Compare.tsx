/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { message as Message } from 'antd';
import { runInAction } from 'mobx';
import i18n from 'ascend-i18n';
import connector from '@/connection';
import { GLOBAL_HOST } from '@/centralServer/websocket/defs';
import type { File } from '@/entity/session';
import { store } from '@/store';
import { cancelBaseline, setBaseline } from '@/utils/Request';
import { sendClusterBaselineStatus } from '@/connection/sendNotification';
import { notNull } from 'ascend-utils';
import { getRankInfo } from '@/utils/Rank';

export interface CompareData extends File {
    rankId: string;
    host?: string;
    cardName?: string;
    isCluster?: boolean;
}

export interface TimelineCard {
    cardName: string;
    cardPath: string;
    host: string;
    rankId: string;
    result: boolean;
}

const sendTabAddBaseline = function(dataSource: any, baseLine: TimelineCard[]): void {
    connector.send({
        event: 'baseline/add',
        body: { dataSource, baseLine },
    });
};

export const sendTabRemoveBaseline = function(dataSource: any, singleDataPath: string): void {
    connector.send({
        event: 'baseline/remove',
        body: { dataSource, singleDataPath },
    });
};

/**
 * 设置基线数据
 * @param projectName 工程名
 * @param fileType 文件类型
 * @param filePath 路径
 */
export const setBaselineData = async ({ projectName, fileType, filePath }: File): Promise<void> => {
    const session = store.sessionStore.activeSession;
    // 取消原来的基线数据
    cancelBaselineData();

    // 设置新的基线
    // 通知后台
    const result: any = await setBaseline({ projectName, fileType, filePath });
    // 基线是工程或者集群文件(cluster_analysis_output)，进入集群对比
    const isProject = projectName !== '' && filePath === '';
    const isClusterCompare = isProject || result.isCluster === true;
    if (notNull(result.errorMessage) || notNull(result?.error?.code)) {
        Message.warning(result.errorMessage as string ?? result.error?.code);
    } else if (isClusterCompare) {
        handleClusterCompare({ projectName, filePath, ...result });
    } else {
        const { rankId, cardName, host } = result as CompareData;
        const timelineCard: TimelineCard = {
            rankId,
            result: true,
            cardName: cardName ?? '',
            host: host ?? '',
            cardPath: filePath,
        };
        const dataSourceForTimeline = { ...GLOBAL_HOST, projectName, dataPath: [filePath] };
        // 通知页签
        sendTabAddBaseline(dataSourceForTimeline, [timelineCard]);
        // 选中基线
        runInAction(() => {
            session.compareSet.baseline = { projectName, fileType, filePath, rankId };
        });
    }
};
/**
 * 集群对比
 */
const handleClusterCompare = async(data: CompareData): Promise<void> => {
    const session = store.sessionStore.activeSession;
    const { activeDataSource } = session;
    // 如果没有打开的工程，告警
    if (activeDataSource.projectName === '') {
        Message.warning(i18n.t('Open a Project as Comparison Data', { ns: 'framework' }));
        return;
    }
    // 如果基线没有集群数据，告警
    if (!data.isCluster) {
        Message.warning(i18n.t('No cluster data available in Baseline', { ns: 'framework' }));
        return;
    }
    // 如果对比没有集群数据，告警
    if (!session.isCluster) {
        Message.warning(i18n.t('No cluster data available in Comparison', { ns: 'framework' }));
        return;
    }
    // 选中基线
    runInAction(() => {
        session.compareSet.baseline = { projectName: data.projectName, fileType: data.fileType, filePath: data.filePath, rankId: '' };
    });
    // 发送通知
    sendClusterBaselineStatus(true);
};

export const isInClusterCompare = (): boolean => {
    const session = store.sessionStore.activeSession;
    const { projectName, rankId } = session.compareSet.baseline;
    return projectName !== '' && rankId === '';
};

/**
 * 取消基线数据
 */
export const cancelBaselineData = async (): Promise<void> => {
    const session = store.sessionStore.activeSession;
    const { projectName, filePath } = session.compareSet.baseline;
    // 通知页签
    const dataSourceForTimeline = { ...GLOBAL_HOST, projectName, dataPath: [filePath] };
    sendTabRemoveBaseline(dataSourceForTimeline, filePath);
    sendClusterBaselineStatus(false);
    // 通知后台
    await cancelBaseline();
    // 取消基线文件，同时也取消对比文件
    runInAction(() => {
        session.compareSet = {
            baseline: { projectName: '', fileType: 'UNKNOWN', filePath: '', rankId: '' },
            comparison: { projectName: '', fileType: 'UNKNOWN', filePath: '', rankId: '' },
        };
    });
};

/**
 * 设置对比数据
 * @param projectName 工程名
 * @param fileType 文件类型
 * @param filePath 路径
 */
export const setCompareData = ({ projectName, fileType, filePath }: File): void => {
    const session = store.sessionStore.activeSession;
    const { activeDataSource } = session;
    if (projectName !== activeDataSource.projectName) {
        Message.warning(i18n.t('Set Comparsion Data Out Of Range', { ns: 'framework' }));
        return;
    };
    // 选中对比文件
    if (fileType === 'PROJECT' || fileType === 'CLUSTER') {
        runInAction(() => {
            session.compareSet.comparison = {
                projectName,
                fileType,
                filePath,
                rankId: '',
                host: '',
                cardName: '',
            };
        });
    } else {
        runInAction(() => {
            const rank = getRankInfo({ projectName, filePath });
            session.compareSet.comparison = {
                projectName,
                fileType,
                filePath,
                rankId: rank?.rankId ?? '',
                host: rank?.host ?? '',
                cardName: rank?.cardName ?? '',
            };
        });
    }
};

/**
 * 取消对比数据
 */
export const cancelCompareData = (): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        session.compareSet.comparison = { projectName: '', fileType: 'UNKNOWN', filePath: '', rankId: '' };
    });
    // 发送通知
    sendClusterBaselineStatus(false);
};

/**
 * 更新对比数据
 * @param oldProjectName 原工程名
 * @param newProjectName 新工程名
 */
export const updateProjectNameHandler = (oldProjectName: string, newProjectName: string): void => {
    const session = store.sessionStore.activeSession;
    // 选中对比文件
    runInAction(() => {
        const { compareSet } = session;
        if (compareSet.baseline.projectName === oldProjectName) {
            compareSet.baseline.projectName = newProjectName;
        }
        if (compareSet.comparison.projectName === oldProjectName) {
            compareSet.comparison.projectName = newProjectName;
        }
    });
};
