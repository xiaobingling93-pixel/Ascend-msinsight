import { store } from '../store';
import { runInAction } from 'mobx';
import { NotificationHandler } from './defs';
import { RankInfo } from '../entity/memory';

export const parseMemoryCompletedHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            session.token = data.token as string;
            const memoryResult = data.memoryResult as RankInfo[];
            const isCluster = data.isCluster as boolean;
            if (!isCluster && !session.isCluster) {
                memoryResult.forEach((item) => {
                    if (!session.memoryRankIds.includes(item.rankId) && (item.hasMemory as boolean)) {
                        session.memoryRankIds.push(item.rankId);
                    }
                });
            } else {
                session.memoryRankIds = [];
                memoryResult.forEach((item) => {
                    item.hasMemory && session.memoryRankIds.push(item.rankId);
                });
                if (data.parseEnd as boolean) {
                    session.isClusterMemoryCompletedSwitch = !session.isClusterMemoryCompletedSwitch;
                }
            }
            session.isCluster = isCluster;
        });
    } catch (err) {
        console.error(err);
    }
};

export const removeRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            session.memoryRankIds = [];
            session.isCluster = false;
        });
    } catch (error) {
        console.error(error);
    }
};

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const wakeUpHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            session.isWakeup = !session.isWakeup;
        });
    } catch (error) {
        console.error(error);
    }
};
