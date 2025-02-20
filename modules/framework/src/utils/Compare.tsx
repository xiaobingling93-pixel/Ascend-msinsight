/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { message as Message } from 'antd';
import { runInAction } from 'mobx';
import i18n from 'ascend-i18n';
import connector from '@/connection';
import { type DataSource, LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import type { File } from '@/entity/session';
import { store } from '@/store';
import { cancelBaseline, setBaseline } from '@/utils/Request';
import { sendClusterBaselineStatus } from '@/connection/sendNotification';
import { notNull } from 'ascend-utils';
import { getRankInfo } from '@/utils/Rank';

export interface CompareData {
    projectName: string;
    filePath: string;
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

const sendTabAddBaseline = function(dataSource: DataSource, baseLine: TimelineCard[]): void {
    connector.send({
        event: 'baseline/add',
        body: { dataSource, baseLine },
    });
};

export const sendTabRemoveBaseline = function(dataSource: DataSource, singleDataPath: string): void {
    connector.send({
        event: 'baseline/remove',
        body: { dataSource, singleDataPath },
    });
};

/**
 * 设置基线数据
 * @param projectName 工程名
 * @param filePath 路径
 */
export const setBaselineData = async ({ projectName, filePath }: File): Promise<void> => {
    const session = store.sessionStore.activeSession;
    // 取消原来的基线数据
    cancelBaselineData();

    // 设置新的基线
    // 通知后台
    const result: any = await setBaseline({ projectName, filePath });
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
        const dataSource = { remote: LOCAL_HOST, port: PORT, projectName, dataPath: [filePath] };
        // 通知页签
        sendTabAddBaseline(dataSource, [timelineCard]);
        // 选中基线
        runInAction(() => {
            session.compareSet.baseline = { projectName, filePath, rankId };
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
        session.compareSet.baseline = { projectName: data.projectName, filePath: data.filePath, rankId: '' };
    });
    // 发送通知
    sendClusterBaselineStatus(true);
};

/**
 * 取消基线数据
 */
export const cancelBaselineData = async (): Promise<void> => {
    const session = store.sessionStore.activeSession;
    const { projectName, filePath } = session.compareSet.baseline;
    // 通知页签
    const datasource = { remote: LOCAL_HOST, port: PORT, projectName, dataPath: [filePath] };
    sendTabRemoveBaseline(datasource, filePath);
    sendClusterBaselineStatus(false);
    // 通知后台
    await cancelBaseline();
    // 取消基线文件，同时也取消对比文件
    runInAction(() => {
        session.compareSet = {
            baseline: { projectName: '', filePath: '', rankId: '' },
            comparison: { projectName: '', filePath: '', rankId: '' },
        };
    });
};

/**
 * 设置对比数据
 * @param projectName 工程名
 * @param filePath 路径
 */
export const setCompareData = ({ projectName, filePath }: File): void => {
    const session = store.sessionStore.activeSession;
    const { activeDataSource } = session;
    if (projectName !== activeDataSource.projectName) {
        Message.warning(i18n.t('Set Comparsion Data Out Of Range', { ns: 'framework' }));
        return;
    };
    // 选中对比文件
    runInAction(() => {
        const rank = getRankInfo({ projectName, filePath });
        session.compareSet.comparison = {
            projectName,
            filePath,
            rankId: rank?.rankId ?? '',
            host: rank?.host ?? '',
            cardName: rank?.cardName ?? '',
        };
    });
};

/**
 * 取消对比数据
 */
export const cancelCompareData = (): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        session.compareSet.comparison = { projectName: '', filePath: '', rankId: '' };
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
