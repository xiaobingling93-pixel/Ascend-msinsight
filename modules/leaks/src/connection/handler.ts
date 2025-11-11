/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import { store } from '../store';
import { runInAction } from 'mobx';
import type { NotificationHandler } from './defs';
import i18n from '@insight/lib/i18n';
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
const commonRestore = (session: any): void => {
    session.maxTime = 0;
    session.minTime = 0;
    session.legendSelect = {};
    session.synStartTime = 0;
    session.synEndTime = 0;
};
const funcRestore = (session: any): void => {
    session.funcData = { traces: [], maxTimestamp: 0, minTimestamp: 0 };
    session.threadId = '';
    session.threadOps = [];
    session.searchFunc = [];
    session.funcOptions = [];
    session.maxDepth = 0;
    session.allowTrim = true;
};
const barRestore = (session: any): void => {
    session.deviceId = '';
    session.eventType = '';
    session.deviceIdOpts = [];
    session.typeOpts = [];
    session.allocationData = { allocations: [], maxTimestamp: 0, minTimestamp: 0 };
    session.blockData = { blocks: [], minSize: 0, maxSize: 0, minTimestamp: 0, maxTimestamp: 0 };
    session.firstOffset = 0;
    session.lastOffset = 0;
    session.markLineshow = 'none';
    session.contextMenu = { visible: false, xPos: 0, yPos: 0 };
    session.allowMark = true;
    session.menuItems = [];
    session.firstLastStamps = { first: 0, last: 0 };
    session.threadFlag = false;
};
const sliceRestore = (session: any): void => {
    session.memoryData = { size: 0, name: '', subNodes: [] };
    session.memoryStamp = 0;
};
const blocksDetailsRestore = (session: any): void => {
    session.blocksTableData = [];
    session.blocksTableHeader = [];
    session.blocksCurrentPage = 1;
    session.blocksPageSize = 10;
    session.blocksTotal = 0;
    session.blocksOrder = '';
    session.blocksOrderBy = '';
    session.blocksFilters = {};
    session.blocksRangeFilters = {};
};
const eventsDetailsRestore = (session: any): void => {
    session.eventsRangeFilters = {};
    session.eventsTableData = [];
    session.eventsTableHeader = [];
    session.eventsCurrentPage = 1;
    session.eventsPageSize = 10;
    session.eventsTotal = 0;
    session.eventsOrder = '';
    session.eventsOrderBy = '';
    session.eventsFilters = {};
};
const detailsRestore = (session: any): void => {
    session.tableType = 'blocks';
    session.tableKey = (session.tableKey + 1) % 10;
    session.lazyUsedThreshold = { perT: null, valueT: null };
    session.delayedFreeThreshold = { perT: null, valueT: null };
    session.longIdleThreshold = { perT: null, valueT: null };
    session.onlyInefficient = false;
};
const restore = (session: any): void => {
    commonRestore(session);
    funcRestore(session);
    barRestore(session);
    sliceRestore(session);
    blocksDetailsRestore(session);
    eventsDetailsRestore(session);
    detailsRestore(session);
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
