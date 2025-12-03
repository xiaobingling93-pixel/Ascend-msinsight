/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { runInAction } from 'mobx';
import { getIndexByRankNameAndDeviceId, getRankInfoKey } from '@insight/lib/utils';
import i18n from '@insight/lib/i18n';
import { store } from '../store';
import type { NotificationHandler } from './defs';
import type { CardInfo, CardRankInfo, DirInfo, RankInfo } from '../entity/session';

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const updateSessionHandler: NotificationHandler = (data): void => {
    const session = store.sessionStore.activeSession;
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
            if (key === 'operatorCardInfos') {
                session.allCardInfos = [...data[key] as CardRankInfo[]].sort((a, b) => Number(a.index) - Number(b.index));
            }
        });
        session.renderId = session.renderId++ % 1000;
    });
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

/** framework 过滤器中也有拦截，逻辑一致 {@link parseOperatorSuccessHandler} */
export const parseSuccessHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    if (!session || !Array.isArray(data.rankList)) {
        return;
    }
    const infos: CardRankInfo[] = [...session.allCardInfos];
    const keys = new Set(infos.map(({ rankInfo }) => getRankInfoKey(rankInfo)));
    data.rankList.forEach((rank: RankInfo) => {
        const key = getRankInfoKey(rank);
        if (keys.has(key)) {
            return;
        }
        infos.push({
            rankInfo: rank,
            dbPath: data.dbPath ?? '',
            index: getIndexByRankNameAndDeviceId(rank.rankName, rank.deviceId),
        } as CardRankInfo);
        keys.add(key);
    });
    infos.sort((a, b) => a.index - b.index);
    runInAction(() => {
        session.allCardInfos = infos;
    });
};

export const resetHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.allCardInfos = [];
        session.projectChangedTrigger = !session.projectChangedTrigger;
    });
};

export const deleteCardHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        const deleteIds: Set<string> = new Set((data.info as CardInfo[]).map(({ cardId }) => cardId));
        if (deleteIds.size > 0) {
            session.allCardInfos = session.allCardInfos.filter((item) => !deleteIds.has(item.rankInfo.rankId));
        }
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

export const allSuccessHandler: NotificationHandler = (data): void => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            if (data.isAllPageParsed as boolean) {
                // 为了触发界面的刷新，重新赋值session.dirInfo
                session.dirInfo = { ...session.dirInfo };
            }
        });
    } catch (error) {
        console.error(error);
    }
};
