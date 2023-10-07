import { store } from '../store';
import { CardMetaData } from '../entity/data';
import { runInAction } from 'mobx';
import { handleMap, recursiveExpandUnit } from '../insight/units/unitFunc';
import { setUnitPhaseByCardId } from '../entity/insight';
import { CardUnit } from '../insight/units/AscendUnit';
import { CardInfo } from '../components/ImportSelect';
import { Session } from '../entity/session';
import { NotificationHandler } from './defs';
import connector from '../connection/index';

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
            session.units.forEach((unit) => {
                if ((unit.metadata as CardMetaData).cardId === unitData.unit.metadata.cardId) {
                    handleMap(unitData.unit, (unit.metadata as CardMetaData).dataSource);
                    recursiveExpandUnit(unitData.unit.children ?? [], unit);
                }
            });
            session.startRecordTime = 0;
            if (session.endTimeAll === undefined) {
                session.endTimeAll = unitData.maxTimeStamp;
            } else {
                session.endTimeAll = Math.max(session.endTimeAll, unitData.maxTimeStamp);
            }
            const remoteAttrs = session.remoteAttrs.get(dataSource.remote);
            if (remoteAttrs === undefined) {
                session.remoteAttrs.set(dataSource.remote, { maxTimeStamp: unitData.maxTimeStamp });
            } else {
                remoteAttrs.maxTimeStamp = unitData.maxTimeStamp;
            }
            setUnitPhaseByCardId(unitData.unit.metadata.cardId, session, 'download');
            if (unitData.startTimeUpdated === true) {
                session.simpleCache.clear();
            }
            if (!(session.units.find(item => item.phase === 'analyzing'))) {
                connector.send({
                    event: 'updateSession',
                    body: { parseCompleted: !(session.units.find(item => item.phase === 'analyzing')) },
                });
            }
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
            session.endTimeAll = 1000000000;
            result.result.forEach((item: CardInfo) => {
                const unit = new CardUnit({ dataSource, cardId: item.rankId, cardName: item.cardName });
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
            body: { isCluster: result.isCluster, isReset: result.reset, startTime: 0, endTimeAll: session?.endTimeAll },
        });
    } catch (error) {
        console.error(error);
    }
};

export const removeRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    try {
        const dataSource = getPropFromData(data, 'dataSource') as DataSource;
        const session = store.sessionStore.activeSession as Session;
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
            return metadata.dataSource.remote !== dataSource.remote;
        });
        session.pinnedUnits = session?.pinnedUnits.filter((unit) => {
            const metadata = unit.metadata as any;
            return metadata.dataSource.remote !== dataSource.remote;
        });
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
    } catch (error) {
        console.error(error);
    }
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
        const result = await window.request(dataSource, { command: 'remote/delete', params: { rankId: removeCardIds } });
        if (result?.startTimeUpdated as boolean) {
            session.remoteAttrs.set(dataSource.remote, { maxTimeStamp: result.maxTimeStamp });

            let remoteMaxTimeStamps = 0;
            session.remoteAttrs.forEach((attrs) => {
                remoteMaxTimeStamps = Math.max(<number>attrs.maxTimeStamp, remoteMaxTimeStamps);
            });
            session.endTimeAll = remoteMaxTimeStamps;
        }
    } catch (error) {
        console.error(error);
    }
};

export const setTheme: NotificationHandler = (data): void => {
    window.setTheme(Boolean(data.isDark));
};

export const clusterCompletedHandler: NotificationHandler = (data): void => {
    if (data.parseResult === 'ok') {
        connector.send({
            event: 'updateSession',
            body: { isCluster: true },
        });
    }
};
