/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { store } from '../store';
import type { CardMetaData, ThreadMetaData, ThreadTrace, ThreadTraceRequest } from '../entity/data';
import { runInAction } from 'mobx';
import { handleMap, recursiveExpandUnit } from '../insight/units/unitFunc';
import { setUnitPhaseByCardId, setUnitProgressByFileId } from '../entity/insight';
import type { InsightUnit } from '../entity/insight';
import { CardUnit, ROOT_UNIT, ThreadUnit } from '../insight/units/AscendUnit';
import type { CardInfo } from '../components/ImportSelect';
import { Session } from '../entity/session';
import type { NotificationHandler } from './defs';
import connector from '../connection/index';
import { message } from 'antd';
import { getTimeOffset, getTimeOffsetKey } from '../insight/units/utils';
import { calculateDomainRange } from '../components/CategorySearch';
import i18n from 'ascend-i18n';
import { forEach, groupBy, isEmpty, cloneDeep } from 'lodash';
import { savePageSetting, recoverPageSetting, updatePageSetting } from '../utils/PageSetting';
import { customConsole as console } from 'ascend-utils';
const DEFAULT_EXPAND_UNIT_NUMBER = 1;
const getPropFromData = function <T extends keyof U, U extends Record<string, unknown>>(data: U, key: T): U[T] {
    if (data[key] === undefined) {
        console.warn(`cannot get ${key.toString()} of `, data);
        throw new Error('missed key');
    }
    return data[key];
};
export const parseSuccessHandler: NotificationHandler = (data): void => {
    try {
        const unitData = data as any;
        const dataSource = getPropFromData(data, 'dataSource') as DataSource;
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            session.isFullDb = unitData.isFullDb;
            // parse suceess之后关闭进度条
            setUnitProgressByFileId(unitData, session);
            session.units.forEach((unit) => {
                if ((unit.metadata as CardMetaData).cardId === unitData.unit.metadata.cardId) {
                    unit.alignStartTimestamp = unitData.offset as number;
                    const prevObj = session.unitsConfig.offsetConfig.timestampOffset;
                    if (unitData.unit.children !== undefined && unitData.unit.children.length > 0) {
                        for (const item of unitData.unit.children) {
                            const key = getTimeOffsetKey(session, item.metadata);
                            session.unitsConfig.offsetConfig.timestampOffset[key] = unit.alignStartTimestamp;
                        }
                    }
                    session.unitsConfig.offsetConfig.timestampOffset = { ...prevObj, [(unit.metadata as CardMetaData).cardId]: unit.alignStartTimestamp };
                    unit.alignStartTimestamp = unitData.offset;
                    handleMap(unitData.unit, (unit.metadata as CardMetaData).dataSource);
                    recursiveExpandUnit(unitData.unit.children ?? [], unit);
                }
            });
            session.startRecordTime = 0;
            const defaultEndTimeAll = (typeof unitData.maxTimeStamp === 'number' ? Math.min(Number.MAX_SAFE_INTEGER, unitData.maxTimeStamp) : 1000000000) * 2;
            if (session.endTimeAll === undefined) {
                session.endTimeAll = defaultEndTimeAll;
            } else {
                session.endTimeAll = Math.max(session.endTimeAll, defaultEndTimeAll);
            }
            const remoteAttrs = session.remoteAttrs.get(dataSource.remote);
            if (remoteAttrs === undefined) {
                session.remoteAttrs.set(dataSource.remote, { maxTimeStamp: defaultEndTimeAll });
            } else {
                remoteAttrs.maxTimeStamp = defaultEndTimeAll;
            }
            setUnitPhaseByCardId(unitData.unit.metadata.cardId, session, 'download');
            if (unitData.startTimeUpdated === true) {
                session.simpleCache.clear();
            }
            session.domainRange = { domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration };

            // 所有卡解析完成
            const parseCompleted = !(session.units.find(item => item.phase === 'analyzing'));
            if (parseCompleted) {
                connector.send({
                    event: 'updateSession',
                    body: {
                        parseCompleted,
                        isFullDb: session.isFullDb,
                    },
                });
                // 恢复上次页面
                recoverPageSetting();
            }
        });
    } catch (error) {
        console.error(error);
    }
};
export const parseProgressHandler: NotificationHandler = (data): void => {
    try {
        const unitData = data as any;
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            session.units.forEach((unit) => {
                if ((unit.metadata as CardMetaData).cardId === unitData.fileId) {
                    unit.progress = unitData.progress;
                    unit.showProgress = true;
                }
            });
        });
    } catch (error) {
        console.error(error);
    }
};
export const parseFailHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        setUnitPhaseByCardId((data as any).rankId, session, 'error');
    });
    message.error(data.error as string);
};

const getRootUnit = (session: Session, host: string, dataSource: DataSource): InsightUnit | undefined => {
    if (isEmpty(host)) {
        return undefined;
    }
    const insightUnit = session.units.find(unit => (unit.metadata as CardMetaData)?.cardId.includes(host))?.parent;
    return insightUnit !== undefined ? insightUnit : new ROOT_UNIT({ dataSource, host });
};

const initUnitSessionInfo = (session: Session, result: any, dataSource: DataSource): void => {
    session.projectName = dataSource.projectName;
    session.phase = 'download';
    session.endTimeAll = undefined;
    session.isPending = result.isPending as boolean;
    session.isParserLoading = !(result.isPending as boolean);
    session.isSimulation = result.isSimulation;
    session.isIpynb = result.isIpynb;
    session.isCluster = result.isCluster;
};

const initUnitInfo = (session: Session | undefined, result: any, dataSource: DataSource): void => {
    if (!session) {
        return;
    }
    if (result.reset as boolean) {
        resetPage({ dataSource });
    }
    initUnitSessionInfo(session, result, dataSource);
    const hostInfo = groupBy(result.result, (item: CardInfo) => item.host ?? '');
    forEach(hostInfo, (cards, host) => {
        const unit = getRootUnit(session, host, dataSource);
        const cardUnits: InsightUnit[] = [];
        if (unit?.children !== undefined) {
            cardUnits.push(...unit.children);
        }
        forEach(cards, (item: CardInfo) => {
            const oldUnit = session.units.find(unitItem => (unitItem.metadata as CardMetaData)?.cardId === item.rankId);
            if (oldUnit !== undefined) {
                return;
            }
            const curDataSource = cloneDeep(dataSource);
            curDataSource.dataPath = item.dataPathList;
            const cardUnit = new CardUnit({ dataSource: curDataSource, cardId: item.rankId, cardName: item.cardName, cardPath: item.cardPath });
            if (item.result as boolean) {
                cardUnit.isParseLoading = !(result.isPending as boolean);
                cardUnit.shouldParse = item.cardName !== 'Host';
                cardUnit.phase = 'analyzing';
                cardUnit.progress = 0;
                cardUnit.showProgress = true;
            } else {
                cardUnit.phase = 'error';
            }
            if (session.units.length < DEFAULT_EXPAND_UNIT_NUMBER) {
                cardUnit.isExpanded = true;
            }
            cardUnits.push(cardUnit);
            session.units = session.units.concat([cardUnit]);
        });
        if (unit) {
            unit.isExpanded = cardUnits.length > 0 ? cardUnits[0].isExpanded : false;
            unit.children = cardUnits;
        }
    });
    session.sortUnits();
};

const createBaselineCard = (session: Session | undefined, result: any, dataSource: DataSource): void => {
    if (!session) {
        return;
    }
    const singleDataPath = dataSource.dataPath[0];
    const isSamePath = session?.units.some((unit) => {
        const metadata = unit.metadata as any;
        if ((metadata.dataSource.dataPath as string[]) === undefined) {
            return false;
        }
        return metadata.dataSource.remote !== dataSource.remote || (metadata.dataSource.dataPath as string[]).includes(singleDataPath);
    });
    if (isSamePath) {
        return;
    }
    session.phase = 'download';
    const hostInfo = groupBy(result, (item: CardInfo) => item.host ?? '');
    forEach(hostInfo, (cards, host) => {
        const unit = isEmpty(host) ? undefined : new ROOT_UNIT({ dataSource, host });
        const cardUnits: InsightUnit[] = [];
        forEach(cards, (item: CardInfo) => {
            const curDataSource = cloneDeep(dataSource);
            curDataSource.dataPath = item.dataPathList;
            const cardUnit = new CardUnit({ dataSource: curDataSource, cardId: item.rankId, cardName: item.cardName, cardPath: item.cardPath });
            if (item.result as boolean) {
                cardUnit.shouldParse = false;
                cardUnit.phase = 'analyzing';
                cardUnit.progress = 0;
                cardUnit.showProgress = true;
            } else {
                cardUnit.phase = 'error';
            }
            if (session.units.length < DEFAULT_EXPAND_UNIT_NUMBER) {
                cardUnit.isExpanded = true;
            }
            cardUnits.push(cardUnit);
            session.units.push(cardUnit);
        });
        if (unit) {
            unit.isExpanded = cardUnits.length > 0 ? cardUnits[0].isExpanded : false;
            unit.children = cardUnits;
        }
    });
    session.sortUnits();
};
export const importRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const dataSource = getPropFromData(data, 'dataSource') as DataSource;
        const projectAction = getPropFromData(data, 'action');
        const isConflict = getPropFromData(data, 'isConflict');
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        if (!session) {
            return;
        }
        // 导入新Remote前，保存当前页面
        // 所有卡解析完成
        const parseCompleted = !(session.units.some(item => item.phase !== 'download'));
        if (parseCompleted) {
            savePageSetting();
        }
        const result = await window.request(dataSource, {
            command: 'import/action',
            params: { projectName: dataSource.projectName, path: dataSource.dataPath, projectAction, isConflict },
        });
        runInAction(() => {
            initUnitInfo(session, result, dataSource);
        });
        sendSessionUpdate(result, session);
    } catch (error) {
        console.error(error);
    }
};

export const baselineAddHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const dataSource = getPropFromData(data, 'dataSource') as DataSource;
        const baseLineInfo = getPropFromData(data, 'baseLine');
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            createBaselineCard(session, baseLineInfo, dataSource);
        });
    } catch (error) {
        console.error(error);
    }
};

const sendSessionUpdate = (result: any, session: any): void => {
    connector.send({
        event: 'updateSession',
        body: {
            isCluster: result.isCluster,
            isReset: result.reset,
            isIpynb: result.isIpynb,
            ipynbUrl: '',
            startTime: 0,
            endTimeAll: session?.endTimeAll,
            unitcount: result.result?.length ?? 0,
            isBinary: result.isBinary,
            coreList: result.coreList,
            sourceList: result.sourceList,
        },
    });
};

const clearIpynbInfo = (session: Session): void => {
    session.isIpynb = false;
    session.ipynbUrl = '';
    connector.send({
        event: 'updateSession',
        body: { isIpynb: session.isIpynb, ipynbUrl: session.ipynbUrl },
    });
};

export const removeRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    resetPage(data);
    updatePageSetting({ type: 'removeDataSource', data: getPropFromData(data, 'dataSource') });
};

export const resetRemoteHandler: NotificationHandler = async (): Promise<void> => {
    resetPage();
    updatePageSetting({ type: 'reset' });
};

const clearUnits = (session: Session, data?: Record<string, unknown>): void => {
    let dataSource: DataSource | null = null;
    if (data) {
        dataSource = getPropFromData(data, 'dataSource') as DataSource;
    }
    if (dataSource !== null && dataSource !== undefined) {
        const removeUnits = session.pinnedUnits.concat(session.units).filter((unit) => {
            const metadata = unit.metadata as any;
            if (metadata.dataSource.dataPath === undefined) {
                return true;
            }
            const isSameDataPath = dataSource?.dataPath.filter((item) => metadata.dataSource.dataPath.includes(item)).length !== 0;
            return metadata.dataSource.remote === dataSource?.remote && isSameDataPath;
        });
        for (const unit of removeUnits) {
            const metadata = unit.metadata as any;
            session.remoteAttrs.delete(metadata.dataSource.remote);
        }
        session.units = session?.units.filter((unit) => {
            const metadata = unit.metadata as any;
            return metadata.dataSource.remote !== dataSource?.remote && !removeUnits.includes(unit);
        });
        session.pinnedUnits = session?.pinnedUnits.filter((unit) => {
            const metadata = unit.metadata as any;
            return metadata.dataSource.remote !== dataSource?.remote && !removeUnits.includes(unit);
        });
    } else {
        session.remoteAttrs.clear();
        session.units = [];
        session.pinnedUnits = [];
    }
};

const resetPage = (data?: Record<string, unknown>): void => {
    runInAction(() => {
        const session = store.sessionStore.activeSession;
        if (!session) {
            return;
        }
        session.isFullDb = false;
        clearUnits(session, data);
        session.simpleCache.clear();
        let remoteMaxTimeStamps = 0;
        session.remoteAttrs.forEach((attrs) => {
            remoteMaxTimeStamps = Math.max(<number>attrs.maxTimeStamp, remoteMaxTimeStamps);
        });
        session.endTimeAll = remoteMaxTimeStamps;
        if (session.selectedUnits[0] !== undefined && !session.units.includes(session.selectedUnits[0])) {
            session.selectedUnits = [];
        }
        if (session.eventUnits[0] !== undefined) {
            session.eventUnits = [];
        }
        if (session.units.length === 0) {
            session.selectedRange = undefined;
        }
        clearIpynbInfo(session);
        clearTimeMarkerFlags(session);
        resetSession();
    });
};

function resetSession(): void {
    runInAction(() => {
        const defaultSessionData = new Session();
        const session = store.sessionStore.activeSession as Session;
        Object.assign(session, {
            doReset: !session.doReset,
            memoryRankIds: [],
            operatorRankIds: [],
            expandedUnitKeys: [],
            contextMenu: defaultSessionData.contextMenu,
            unitsConfig: defaultSessionData.unitsConfig,
            scrollTop: 0,
            selectedDetailKeys: [],
            selectedDetails: [],
            searchData: undefined,
        });
        session.selectedUnitKeys = [];
        session.singleLinkLine = {};
    });
}

const clearTimeMarkerFlags = (session: Session): void => {
    session.timelineMaker.selectedFlag = undefined;
    session.timelineMaker.refreshTrigger = (session.timelineMaker.refreshTrigger + 1) % 10;
    session.timelineMaker.timelineFlagList.splice(0);
};

const remoteDeleteRequest = async (session: Session, dataSource: DataSource, removeCardIds: any[]): Promise<void> => {
    const result = await window.request(dataSource, { command: 'remote/delete', params: { rankId: removeCardIds } });
    if (result?.startTimeUpdated as boolean) {
        session.remoteAttrs.set(dataSource.remote, { maxTimeStamp: result.maxTimeStamp });

        let remoteMaxTimeStamps = 0;
        session.remoteAttrs.forEach((attrs) => {
            remoteMaxTimeStamps = Math.max(<number>attrs.maxTimeStamp, remoteMaxTimeStamps);
        });
        session.endTimeAll = remoteMaxTimeStamps;
    }
};

export const removeBaselineHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        runInAction(() => {
            const dataSource = getPropFromData(data, 'dataSource') as DataSource;
            const singleDataPath = getPropFromData(data, 'singleDataPath') as string;
            const session = store.sessionStore.activeSession as Session;
            const removeUnits = getRemoveUnits(session, dataSource, singleDataPath);
            session.units = session?.units.filter((unit) => {
                const metadata = unit.metadata as any;
                if (!((metadata.cardName.startsWith('baseline')) as boolean)) {
                    return true;
                }
                if ((metadata.dataSource.dataPath as string[]) === undefined) {
                    return false;
                }
                return metadata.dataSource.remote !== dataSource.remote || !(metadata.dataSource.dataPath as string[]).includes(singleDataPath);
            });
            if (session.selectedUnits[0] !== undefined && !session.units.includes(session.selectedUnits[0])) {
                session.selectedUnits = [];
            }
            if (session.units.length === 0) {
                session.selectedRange = undefined;
            }
            if (session.eventUnits[0] !== undefined) {
                session.eventUnits = [];
            }
            session.doReset = !session.doReset;
            for (const unit of removeUnits) {
                const metadata = unit.metadata as any;
                const remote = metadata.dataSource.remote;
                if (session.remoteAttrs.has(remote) && !session.units.find(item => (item.metadata as any)?.dataSource.remote === remote)) {
                    session.remoteAttrs.delete(remote);
                }
            }
            session.pinnedUnits = session?.pinnedUnits.filter((unit) => {
                const metadata = unit.metadata as any;
                return metadata.dataSource.remote !== dataSource.remote || !(metadata.dataSource.dataPath as string[]).includes(singleDataPath);
            });
            clearTimeMarkerFlags(session);
        });
    } catch (error) {
        console.error(error);
    }
};

export const removeSingleRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const dataSource = getPropFromData(data, 'dataSource') as DataSource;
        const singleDataPath = getPropFromData(data, 'singleDataPath') as string;
        const session = store.sessionStore.activeSession as Session;
        const removeUnits = getRemoveUnits(session, dataSource, singleDataPath);
        const removeCardIds = getRemoveCardIds(removeUnits);
        session.units = session?.units.filter((unit) => {
            const metadata = unit.metadata as any;
            if ((metadata.dataSource.dataPath as string[]) === undefined) {
                return true;
            }
            return metadata.dataSource.remote !== dataSource.remote || !(metadata.dataSource.dataPath as string[]).includes(singleDataPath);
        });
        if (session.selectedUnits[0] !== undefined && !session.units.includes(session.selectedUnits[0])) {
            session.selectedUnits = [];
        }
        if (session.units.length === 0) {
            session.selectedRange = undefined;
        }
        if (session.eventUnits[0] !== undefined) {
            session.eventUnits = [];
        }
        session.doReset = !session.doReset;
        for (const unit of removeUnits) {
            const metadata = unit.metadata as any;
            const remote = metadata.dataSource.remote;
            if (session.remoteAttrs.has(remote) && !session.units.find(item => (item.metadata as any)?.dataSource.remote === remote)) {
                session.remoteAttrs.delete(remote);
            }
        }
        session.pinnedUnits = session?.pinnedUnits.filter((unit) => {
            const metadata = unit.metadata as any;
            return metadata.dataSource.remote !== dataSource.remote || !(metadata.dataSource.dataPath as string[]).includes(singleDataPath);
        });
        await remoteDeleteRequest(session, dataSource, removeCardIds);
        clearTimeMarkerFlags(session);
        clearIpynbInfo(session);
        connector.send({ event: 'deleteRank', body: { rankId: removeCardIds } });
        if (removeCardIds.length > 0) {
            const memoryRankIds = session.memoryRankIds.filter((item: string) => !removeCardIds?.includes(item));
            session.memoryRankIds = memoryRankIds;
            const operatorRankIds = session.operatorRankIds.filter((item: string) => !removeCardIds?.includes(item));
            session.operatorRankIds = operatorRankIds;
            connector.send({ event: 'updateSession', body: { operatorRankIds, memoryRankIds, broadcast: false } });
        }
        // 更新已保存的页面设置
        updatePageSetting({ type: 'removeSingleDataPath', data });
    } catch (error) {
        console.error(error);
    }
    connector.send({ event: 'updateSession', body: { loading: false } });
};

const getRemoveUnits = (session: Session, dataSource: DataSource, singleDataPath: string): InsightUnit[] => {
    return session.units.filter((unit) => {
        const metadata = unit.metadata as any;
        if ((metadata.dataSource.dataPath as string[]) === undefined) {
            return true;
        }
        const isSameDataPath = metadata.dataSource.dataPath.includes(singleDataPath);
        return metadata.dataSource.remote === dataSource.remote && isSameDataPath;
    });
};

const getRemoveCardIds = (removeUnits: any[]): any[] => {
    return removeUnits.map((unit) => {
        const metadata = unit.metadata as any;
        return metadata.cardId;
    });
};

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const clusterCompletedHandler: NotificationHandler = (data): void => {
    const clusterRes = (data?.parseResult === 'ok' || data?.parseResult === 'none');
    const isShowCluster = data?.isShowCluster as boolean;
    const session = store.sessionStore.activeSession as Session;
    session.isCluster = isShowCluster;
    const clusterCompleted = isShowCluster && clusterRes;
    connector.send({
        event: 'updateSession',
        body: { clusterCompleted },
    });
};

export const clusterDurationCompletedHandler: NotificationHandler = (data): void => {
    const clusterRes = data?.parseResult === 'ok';
    connector.send({
        event: 'updateSession',
        body: { isCluster: clusterRes, durationFileCompleted: clusterRes },
    });
};

export const locateUnitHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession as Session;
    const slice = data as ThreadTrace;
    runInAction(() => {
        session.locateUnit = {
            target: (unit: InsightUnit): boolean => {
                return unit instanceof ThreadUnit && (Boolean(unit.metadata.cardId.includes(slice.rankId))) &&
                    unit.metadata.processId === slice.processId && unit.metadata.threadId === slice.threadId;
            },
            onSuccess: (unit): void => {
                const startTime = slice.startTime - getTimeOffset(session, unit.metadata as ThreadMetaData);
                const [rangeStart, rangeEnd] = calculateDomainRange(session, startTime, slice.duration);
                session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                session.selectedData = {
                    id: slice.id,
                    startTime,
                    duration: slice.duration,
                    depth: slice.depth,
                    threadId: slice.threadId,
                    metaType: (unit.metadata as ThreadMetaData).metaType,
                };
            },
            showDetail: true,
        };
    });
};

export const jupyterCompletedHandler: NotificationHandler = (data): void => {
    const isIpynb = data?.parseResult === 'ok';
    const session = store.sessionStore.activeSession as Session;
    session.isIpynb = isIpynb;
    session.ipynbUrl = data.url as string;
    connector.send({
        event: 'updateSession',
        body: { isIpynb, ipynbUrl: data.url as string },
    });
    if (!isIpynb) {
        message.error('Jupyter launch error!');
    }
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

export const parseMemorySuccessHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        const memoryResult = data.memoryResult as Array<{ rankId: string;hasMemory: boolean }>;
        const memoryRankIds: string[] = [];
        memoryResult.forEach(item => {
            if (item.hasMemory) {
                memoryRankIds.push(item.rankId);
            }
        });
        session.memoryRankIds = memoryRankIds;
        connector.send({ event: 'updateSession', body: { memoryRankIds, broadcast: false } });
    });
};

export const parseOperatorSuccessHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        const ids = [...session.operatorRankIds, String(data.rankId)].sort((a, b) => Number(a) - Number(b));
        session.operatorRankIds = ids;
        connector.send({ event: 'updateSession', body: { operatorRankIds: session.operatorRankIds, broadcast: false } });
    });
};

interface CardOffset {
    cardId: string;
    offset: number;
}
export const allSuccessHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        const cardOffSets = data.cardOffsets as CardOffset[];
        const offsetMap: Map<string, number> = new Map();
        cardOffSets.forEach((value) => {
            offsetMap.set(value.cardId, value.offset);
        });
        runInAction(() => {
            if (!session) {
                return;
            }
            if (data.isAllPageParsed as boolean) {
                session.isPending = false;
                session.isParserLoading = false;
            }
            session.units.forEach((unit) => {
                unit.alignStartTimestamp = offsetMap.get((unit.metadata as CardMetaData).cardId);
                const prevObj = session.unitsConfig.offsetConfig.timestampOffset;
                if (unit.alignStartTimestamp !== undefined) {
                    if (unit.children !== undefined && unit.children.length > 0) {
                        for (const item of unit.children) {
                            const key = getTimeOffsetKey(session, item.metadata as ThreadTraceRequest);
                            session.unitsConfig.offsetConfig.timestampOffset[key] = unit.alignStartTimestamp;
                        }
                    }
                    session.unitsConfig.offsetConfig.timestampOffset = { ...prevObj, [(unit.metadata as CardMetaData).cardId]: (unit.alignStartTimestamp) };
                }
            });
        });
    } catch (error) {
        console.error(error);
    }
};

export const updateProjectNameHandler: NotificationHandler = (data): void => {
    const session = store.sessionStore.activeSession;
    if (session === undefined) {
        return;
    }
    const { oldProjectName, newProjectName } = (data ?? {}) as Record<string, string>;
    if (session.projectName === oldProjectName) {
        session.projectName = newProjectName;
    }
    updatePageSetting({ type: 'updateProjectName', data });
};
