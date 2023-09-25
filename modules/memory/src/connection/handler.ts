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
            memoryResult.forEach((item) => {
                if (!session.memoryRankIds.includes(item.rankId) && (item.hasMemory as boolean)) {
                    session.memoryRankIds.push(item.rankId);
                }
            });
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
        });
    } catch (error) {
        console.error(error);
    }
};

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};
