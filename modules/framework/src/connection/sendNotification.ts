/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import connector from '@/connection/index';
import { themeInstance } from 'ascend-theme';
import { store } from '@/store';
import { localStorageService, LocalStorageKey } from 'ascend-utils';
import { ThemeName, Language } from '@/utils/enum';

export function sendTheme(): void {
    connector.send({
        event: 'setTheme',
        body: { isDark: themeInstance.getCurrentTheme() === ThemeName.DARK },
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
