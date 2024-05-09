import { store } from '../store';
import type { CardMetaData, ThreadMetaData, ThreadTrace } from '../entity/data';
import { runInAction } from 'mobx';
import { handleMap, recursiveExpandUnit } from '../insight/units/unitFunc';
import { setUnitPhaseByCardId } from '../entity/insight';
import type { InsightUnit } from '../entity/insight';
import { CardUnit, ThreadUnit } from '../insight/units/AscendUnit';
import { CardInfo } from '../components/ImportSelect';
import { Session } from '../entity/session';
import { NotificationHandler } from './defs';
import connector from '../connection/index';
import { message } from 'antd';
import { getTimeOffset } from '../insight/units/utils';
import { calculateDomainRange } from '../components/CategorySearch';

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

export const parseFailHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        setUnitPhaseByCardId((data as any).rankId, session, 'error');
    });
    message.error(data.error);
};

export const importRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const dataSource = getPropFromData(data, 'dataSource') as DataSource;
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        const result = await window.request(dataSource, { command: 'import/action', params: { path: dataSource.dataPath } });
        runInAction(() => {
            if (!session) {
                return;
            }
            (result.reset as boolean) && (session.units = []);
            session.phase = 'download';
            session.endTimeAll = undefined;
            session.isSimulation = result.isSimulation;
            session.isCluster = result.isCluster;
            result.result.forEach((item: CardInfo) => {
                const unit = new CardUnit({ dataSource, cardId: item.rankId, cardName: item.cardName, cardPath: item.cardPath });
                if (item.result as boolean) {
                    unit.phase = 'analyzing';
                } else {
                    unit.phase = 'error';
                }
                session.units.push(unit);
            });
            session.sortUnits();
        });
        connector.send({
            event: 'updateSession',
            body: {
                isCluster: result.isCluster,
                isReset: result.reset,
                startTime: 0,
                endTimeAll: session?.endTimeAll,
                unitcount: result.result?.length ?? 0,
                isBinary: result.isBinary,
                coreList: result.coreList,
                sourceList: result.sourceList,
            },
        });
    } catch (error) {
        console.error(error);
    }
};

export const removeRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
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
        if (session.units.length === 0) {
            session.selectedRange = undefined;
        }
        clearTimeMarkerFlags(session);
    } catch (error) {
        console.error(error);
    }
};

export const dragImportSuccessHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const dataSource = getPropFromData(data, 'dataSource') as DataSource;
        const result: any = getPropFromData(data, 'result');
        const { sessionStore } = store;
        const session = sessionStore.activeSession;

        if (result.result === undefined || result.result === null) {
            return;
        }
        runInAction(() => {
            if (!session) {
                return;
            }
            (result.reset as boolean) && (session.units = []);
            session.phase = 'download';
            session.endTimeAll = undefined;
            result.result.forEach((item: CardInfo) => {
                const unit = new CardUnit({ dataSource, cardId: item.rankId, cardName: item.cardName, cardPath: item.cardPath });
                if (item.result as boolean) {
                    unit.phase = 'analyzing';
                } else {
                    unit.phase = 'error';
                }
                session.units.push(unit);
            });
        });
        connector.send({
            event: 'updateSession',
            body: {
                isCluster: result.isCluster,
                isReset: result.reset,
                startTime: 0,
                endTimeAll: session?.endTimeAll,
                unitcount: result.result?.length ?? 0,
            },
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

export const removeSingleRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const dataSource = getPropFromData(data, 'dataSource') as DataSource;
        const singleDataPath = getPropFromData(data, 'singleDataPath') as string;
        const session = store.sessionStore.activeSession as Session;

        const removeUnits = session.units.filter((unit) => {
            const metadata = unit.metadata as any;
            const isSameDataPath = metadata.dataSource.dataPath.includes(singleDataPath);
            return metadata.dataSource.remote === dataSource.remote && isSameDataPath;
        });

        const removeCardIds = removeUnits.map((unit) => {
            const metadata = unit.metadata as any;
            return metadata.cardId;
        });
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
        for (const unit of removeUnits) {
            const metadata = unit.metadata as any;
            const remote = metadata.dataSource.remote;
            if (session.remoteAttrs.has(remote) && !session.units.find(unit => (unit.metadata as any)?.dataSource.remote === remote)) {
                session.remoteAttrs.delete(remote);
            }
        }
        session.pinnedUnits = session?.pinnedUnits.filter((unit) => {
            const metadata = unit.metadata as any;
            return metadata.dataSource.remote !== dataSource.remote || !(metadata.dataSource.dataPath as string[]).includes(singleDataPath);
        });
        const result = await window.request(dataSource, { command: 'remote/delete', params: { rankId: removeCardIds } });
        if (result?.startTimeUpdated as boolean) {
            session.remoteAttrs.set(dataSource.remote, { maxTimeStamp: result.maxTimeStamp });

            let remoteMaxTimeStamps = 0;
            session.remoteAttrs.forEach((attrs) => {
                remoteMaxTimeStamps = Math.max(<number>attrs.maxTimeStamp, remoteMaxTimeStamps);
            });
            session.endTimeAll = remoteMaxTimeStamps;
        }
        clearTimeMarkerFlags(session);
        connector.send({ event: 'deleteRank', body: { rankId: removeCardIds } });
    } catch (error) {
        console.error(error);
    }
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
