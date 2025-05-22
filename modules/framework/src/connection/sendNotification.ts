/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import connector from '@/connection/index';
import { themeInstance } from 'ascend-theme';
import { store } from '@/store';
import { localStorageService, LocalStorageKey } from 'ascend-local-storage';
import { ThemeName, Language } from '@/utils/enum';
import { getRankId } from '@/utils/Rank';
import type { LayerType } from '@/centralServer/websocket/defs';

export function sendTheme(to?: number): void {
    connector.send({
        event: 'setTheme',
        body: { isDark: themeInstance.getCurrentTheme() === ThemeName.DARK },
        to,
    });
}

export function sendModuleReset(): void {
    connector.send({
        event: 'module.reset',
        body: {},
    });
}

// 数据解析状态
export function sendStatus(): void {
    const session = store.sessionStore.activeSession;
    connector.send({
        event: 'updateSession',
        body: {
            parseCompleted: session.parseCompleted,
            clusterPageInfo: session.clusterPageInfo,
            isFullDb: session.isFullDb,
            unitcount: session.unitcount,
            coreList: session.coreList,
            sourceList: session.sourceList,
            isIpynb: session.isIpynb,
            ipynbUrl: session.ipynbUrl,
            instrVersion: session.instrVersion,
        },
    });
}

export function sendLanguage(to?: number): void {
    connector.send({
        event: 'switchLanguage',
        to,
        body: { lang: localStorageService.getItem(LocalStorageKey.LANGUAGE) ?? Language.EN },
    });
}

export function sendWakeupPlugin(): void {
    const session = store.sessionStore.activeSession;
    connector.send({
        event: 'wakeupPlugin',
        data: { url: session.toIframeUrl },
        target: 'plugin',
    });
}

export function sendUpdateProjectName(oldProjectName: string, newProjectName: string): void {
    connector.send({
        event: 'updateProjectName',
        body: { oldProjectName, newProjectName },
    });
}

export function sendReset(): void {
    connector.send({ event: 'remote/reset', body: {}, target: 'plugin' });
}

export const sendClusterBaselineStatus = (status: boolean): void => {
    connector.send({
        event: 'clusterBaselineToggle',
        body: { status },
    });
};

export const sendShortcutKeys = (key: {hasCtrl: boolean;key: string}): void => {
    connector.send({
        event: 'keydownShortcut',
        body: key,
    });
};

export const sendDirectory = (to?: number): void => {
    const session = store.sessionStore.activeSession;
    let rankId: string = '';
    let selectedFileType: LayerType;
    let selectedFilePath: string;
    // 比对数据
    if (session.isCompareStatus) {
        rankId = session.compareSet.comparison.rankId;
        selectedFileType = session.compareSet.comparison.fileType;
        selectedFilePath = session.compareSet.comparison.filePath;
    } else {
        const { projectName } = session.activeDataSource;
        // 切换目录
        if (projectName !== '' && session.activeDataSource.selectedFilePath !== '') {
            rankId = getRankId({ projectName, filePath: session.activeDataSource.selectedFilePath });
        }
        selectedFileType = session.activeDataSource.selectedFileType;
        selectedFilePath = session.activeDataSource.selectedFilePath;
    }
    // 通知页签
    connector.send({
        event: 'switchDirectory',
        body: {
            rankId,
            selectedFileType,
            selectedFilePath,
            // 比对数据存在时，比对数据所属的项目就是 session.activeDataSource
            selectedProjectName: session.activeDataSource.projectName,
            // 比对数据存在时，比对数据所属的项目就是 session.activeDataSource，比对数据的 clusterPageInfo 就是 session.clusterPageInfo
            pageInfo: {
                cluster: session.clusterPageInfo,
                timeline: session.timelinePageInfo,
            },
            isCompare: session.isCompareStatus,
        },
        to,
    });
};

export const sendSessionKey = (key: string, to?: number): void => {
    const session = store.sessionStore.activeSession;
    const value = (session as any)[key];
    if (value === undefined) {
        return;
    }
    connector.send({
        event: 'updateSession',
        body: { [key]: value },
        to,
    });
};

export const sendMap: Record<string, (to?: number) => void> = {
    directory: sendDirectory,
    status: sendStatus,
    theme: sendTheme,
    language: sendLanguage,
    memoryRankIds: (to) => sendSessionKey('memoryRankIds', to),
    operatorRankIds: (to) => sendSessionKey('operatorRankIds', to),
};
