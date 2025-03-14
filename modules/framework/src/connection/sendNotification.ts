/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import connector from '@/connection/index';
import { themeInstance } from 'ascend-theme';
import { store } from '@/store';
import { localStorageService, LocalStorageKey } from 'ascend-local-storage';
import { ThemeName, Language } from '@/utils/enum';
import { getRankId } from '@/utils/Rank';

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
            clusterCompleted: session.clusterCompleted,
            isFullDb: session.isFullDb,
            unitcount: session.unitcount,
            coreList: session.coreList,
            sourceList: session.sourceList,
            durationFileCompleted: session.durationFileCompleted,
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
    const { projectName, dataPath } = session.activeDataSource;
    let rankId: string = '';
    // 比对数据
    if (session.isCompareStatus) {
        rankId = session.compareSet.comparison.rankId;
    } else {
        // 切换目录
        if (projectName !== '' && dataPath.length > 0) {
            rankId = getRankId({ projectName, filePath: dataPath[0] });
        }
    }
    // 通知页签
    connector.send({
        event: 'switchDirectory',
        body: { rankId, isCompare: session.isCompareStatus },
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
