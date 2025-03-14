/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { store } from '../store';
import { runInAction } from 'mobx';
import { type NotificationHandler } from './defs';
import i18n from 'ascend-i18n';
import { type DirInfo, InstructionSelectSource } from '../entity/session';
import { closeFind, openFind } from '../components/hotMethod/CodeTextSearch';
import type { KeydownInfo } from '@/utils/interface';
import { KEYS } from 'ascend-utils';

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        const isDark = Boolean(data.isDark);
        session.theme = isDark ? 'dark' : 'light';
    });
};

export const importRemoteHandler = (): void => {
    reset();
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
        const updateData = {};
        dataKeys.forEach((key: string) => {
            if (sessionKeys.includes(key)) {
                Object.assign(updateData, { [key]: data[key] });
            }
        });
        if (Object.keys(updateData).length > 0) {
            Object.assign(session, {
                ...updateData,
                parseStatus: true,
                updateId: ++session.updateId % 100,
            });
        }
    });
};

export const resetHandler: NotificationHandler = (data): void => {
    reset();
};
const reset = (): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        Object.assign(session, {
            coreList: [],
            sourceList: [],
            parseStatus: false,
            blockIdList: [],
            instructions: [],
        });
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

export const switchDirectoryHandler: NotificationHandler = async (data): Promise<void> => {
    const session = store.sessionStore.activeSession;
    const dirInfo = { rankId: data.rankId, isCompare: data.isCompare } as DirInfo;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.dirInfo = dirInfo;
    });
};

export const keydownHandler: NotificationHandler = async (data): Promise<void> => {
    shortcutSwitchFindWindow(data as any);
};

export const shortcutSwitchFindWindow = (keyInfo: KeydownInfo): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        // ctrl+f键(Mac环境Command+f)，且当打开前source页
        // esc键退出
        const isOnSource = document.URL.toLowerCase().includes('source') && document.getElementById('root')?.offsetParent !== null;
        if (!isOnSource) {
            return;
        }
        const isOpenKey = (keyInfo.isMac ? keyInfo.hasCommand : keyInfo.hasCtrl) && keyInfo.key === KEYS.F;
        const isCloseKey = keyInfo.key === KEYS.ESCAPE;
        if (isOpenKey) {
            openFind();
        }
        if (isCloseKey) {
            closeFind();
        }
    });
};

// 高亮cache 映射指令
export const showCacheInstructionsHandler: NotificationHandler = async (data): Promise<void> => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.cacheUnit = {
            cachelineId: (data as any).cachelineId,
            addressRange: (data as any).addressRange,
        };
        session.instructionSelectSource = InstructionSelectSource.CACHE;
        session.instructionUpdateId = (session.instructionUpdateId + 1) % 100;
    });
};
