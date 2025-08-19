/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import { store } from '../store';
import { runInAction } from 'mobx';
import type { NotificationHandler } from './defs';
import i18n from 'ascend-i18n';
export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const updateSessionHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session || typeof session !== 'object' || typeof data !== 'object') {
            return;
        }
        const dataKeys = Object.keys(data);
        const sessionKeys = Object.keys(session);
        dataKeys.forEach((key: string) => {
            if (sessionKeys.includes(key)) {
                (session as unknown as Record<string, unknown>)[key] = data[key];
            }
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
const restore = (session: any): void => {
    session.allocationData = { allocations: [], maxTimestamp: 0, minTimestamp: 0 };
    session.blockData = { blocks: [], minSize: 0, maxSize: 0, minTimestamp: 0, maxTimestamp: 0 };
    session.memoryData = { size: 0, name: '', subNodes: [] };
    session.funcData = { traces: [], maxTimestamp: 0, minTimestamp: 0 };
    session.deviceId = '';
    session.eventType = '';
    session.threadId = '';
    session.memoryStamp = 0;
    session.deviceIdOpts = [];
    session.typeOpts = [];
    session.threadOps = [];
    session.maxTime = 0;
    session.minTime = 0;
    session.legendSelect = {};
    session.synStartTime = 0;
    session.synEndTime = 0;
    session.searchFunc = [];
    session.funcOptions = [];
    session.threadFlag = false;
    session.maxDepth = 0;
    session.blocksTableData = [];
    session.blocksTableHeader = [];
    session.blocksCurrentPage = 1;
    session.blocksPageSize = 10;
    session.blocksTotal = 0;
    session.blocksOrder = '';
    session.blocksOrderBy = '';
    session.blocksFilters = {};
    session.blocksRangeFilters = {};
    session.eventsRangeFilters = {};
    session.eventsTableData = [];
    session.eventsTableHeader = [];
    session.eventsCurrentPage = 1;
    session.eventsPageSize = 10;
    session.eventsTotal = 0;
    session.eventsOrder = '';
    session.eventsOrderBy = '';
    session.eventsFilters = {};
    session.tableType = 'blocks';
    session.tableKey = (session.tableKey + 1) % 10;
    session.firstOffset = 0;
    session.lastOffset = 0;
    session.markLineshow = 'none';
    session.contextMenu = { visible: false, xPos: 0, yPos: 0 };
    session.allowMark = true;
    session.menuItems = [];
    session.firstLastStamps = { first: 0, last: 0 };
};
export const parseCompletedHandler = (data: any): void => {
    const session = store.sessionStore.activeSession;
    if (session) {
        runInAction(() => {
            session.deviceIds = data.deviceIds;
            session.threadIds = data.threadIds;
            restore(session);
        });
    }
};
export const removeRemoteHandler: NotificationHandler = (data): void => {
    const session = store.sessionStore.activeSession;
    if (session) {
        runInAction(() => {
            session.deviceIds = {};
            session.threadIds = [];
            restore(session);
        });
    }
};
