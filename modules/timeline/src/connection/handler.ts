/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { store } from '../store';
import type { CardMetaData, ThreadMetaData, ThreadTrace } from '../entity/data';
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
import { getTimeOffset } from '../insight/units/utils';
import { calculateDomainRange } from '../components/CategorySearch';
import i18n from 'ascend-i18n';
import { forEach, groupBy, isEmpty, cloneDeep } from 'lodash';
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
            if (!(session.units.find(item => item.phase === 'analyzing'))) {
                connector.send({
                    event: 'updateSession',
                    body: { parseCompleted: !(session.units.find(item => item.phase === 'analyzing')), isFullDb: session.isFullDb },
                });
            }
            runInAction(() => {
                session.domainRange = { domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration };
            });
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

const initUnitInfo = (session: Session | undefined, result: any, dataSource: DataSource): void => {
    if (!session) {
        return;
    }
    if (result.reset as boolean) {
        removeRemoteHandler({ dataSource });
    }

    session.phase = 'download';
    session.endTimeAll = undefined;
    session.isPending = result.isPending as boolean;
    session.isParserLoading = !(result.isPending as boolean);
    session.isSimulation = result.isSimulation;
    session.isIpynb = result.isIpynb;
    session.isCluster = result.isCluster;
    const hostInfo = groupBy(result.result, (item: CardInfo) => item.host ?? '');
    forEach(hostInfo, (cards, host) => {
        const unit = isEmpty(host) ? undefined : new ROOT_UNIT({ dataSource, host });
        const cardUnits: InsightUnit[] = [];
        forEach(cards, (item: CardInfo) => {
            const curDataSource = cloneDeep(dataSource);
            curDataSource.dataPath = item.dataPathList;
            const cardUnit = new CardUnit({ dataSource: curDataSource, cardId: item.rankId, cardName: item.cardName, cardPath: item.cardPath });
            if (item.result as boolean) {
                cardUnit.shouldParse = true;
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
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        const result = await window.request(dataSource, { command: 'import/action', params: { projectName: dataSource.projectName, path: dataSource.dataPath } });
        runInAction(() => {
            initUnitInfo(session, result, dataSource);
        });
        sendSessionUpdate(result, session);
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
    const defaultSessionData = new Session();
    try {
        runInAction(() => {
            const dataSource = getPropFromData(data, 'dataSource') as DataSource;
            const session = store.sessionStore.activeSession as Session;
            session.isFullDb = false;
            const removeUnits = session.pinnedUnits.concat(session.units).filter((unit) => {
                const metadata = unit.metadata as any;
                const isSameDataPath = dataSource.dataPath.filter((item) => metadata.dataSource.dataPath.includes(item)).length !== 0;
                return metadata.dataSource.remote === dataSource.remote && isSameDataPath;
            });
            for (const unit of removeUnits) {
                const metadata = unit.metadata as any;
                session.remoteAttrs.delete(metadata.dataSource.remote);
            }
            session.units = session?.units.filter((unit) => {
                const metadata = unit.metadata as any;
                return metadata.dataSource.remote !== dataSource.remote && !removeUnits.includes(unit);
            });
            session.pinnedUnits = session?.pinnedUnits.filter((unit) => {
                const metadata = unit.metadata as any;
                return metadata.dataSource.remote !== dataSource.remote && !removeUnits.includes(unit);
            });
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
        });
    } catch (error) {
        console.error(error);
    }
};

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

export const removeSingleRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const dataSource = getPropFromData(data, 'dataSource') as DataSource;
        const singleDataPath = getPropFromData(data, 'singleDataPath') as string;
        const session = store.sessionStore.activeSession as Session;
        const removeUnits = getRemoveUnits(session, dataSource, singleDataPath);
        const removeCardIds = getRemoveCardIds(removeUnits);
        session.units = session?.units.filter((unit) => {
            const metadata = unit.metadata as any;
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
    } catch (error) {
        console.error(error);
    }
};

const getRemoveUnits = (session: Session, dataSource: DataSource, singleDataPath: string): InsightUnit[] => {
    return session.units.filter((unit) => {
        const metadata = unit.metadata as any;
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
    const clusterRes = data?.parseResult === 'ok';
    const session = store.sessionStore.activeSession as Session;
    session.isCluster = clusterRes;
    connector.send({
        event: 'updateSession',
        body: { isCluster: clusterRes, clusterCompleted: clusterRes },
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
                const startTime = slice.startTime - getTimeOffset(session, (unit.metadata as ThreadMetaData).cardId);
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

export const allSuccessHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        runInAction(() => {
            if (!session) {
                return;
            }
            if (data.isAllPageParsed as boolean) {
                session.isPending = false;
                session.isParserLoading = false;
            }
        });
    } catch (error) {
        console.error(error);
    }
};
