import { store } from '../store';
import { CardMetaData } from '../entity/data';
import { runInAction } from 'mobx';
import { handleMap, recursiveExpandUnit } from '../insight/units/unitFunc';
import { setUnitPhaseByCardId } from '../entity/insight';
import { CardUnit } from '../insight/units/AscendUnit';
import { CardInfo } from '../components/ImportSelect';
import { Session } from '../entity/session';
import { NotificationHandler } from './defs';

export const parseSuccessHandler: NotificationHandler = (data): void => {
    const unitData = data.body as any;
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
        const remoteAttrs = session.remoteAttrs.get(data.dataSource.remote);
        if (remoteAttrs === undefined) {
            session.remoteAttrs.set(data.dataSource.remote, { maxTimeStamp: unitData.maxTimeStamp });
        } else {
            remoteAttrs.maxTimeStamp = unitData.maxTimeStamp;
        }
        setUnitPhaseByCardId(unitData.unit.metadata.cardId, session, 'download');
        if (unitData.startTimeUpdated === true) {
            session.simpleCache.clear();
        }
    });
};

export const parseFailHandler: NotificationHandler = (data): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        setUnitPhaseByCardId((data.body as any).rankId, session, 'error');
    });
};

export const importRemoteHandler: NotificationHandler = async (data): Promise<void> => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    const result = await window.request(data.dataSource, { command: 'import/action', params: { path: data.dataSource.dataPath } });
    runInAction(() => {
        if (!session) {
            return;
        }
        session.phase = 'download';
        session.endTimeAll = 1000000000;
        result.result.forEach((item: CardInfo) => {
            const unit = new CardUnit({ dataSource: data.dataSource, cardId: item.rankId, cardName: item.cardName });
            if (item.result as boolean) {
                unit.phase = 'analyzing';
            } else {
                unit.phase = 'error';
            }
            session.units.push(unit);
        });
    });
};

export const removeRemoteHandler = async ({ dataSource }: any): Promise<void> => {
    const session = store.sessionStore.activeSession as Session;
    const removeUnits = session.units.filter((unit) => {
        const metadata = unit.metadata as any;
        return metadata.dataSource.remote === dataSource.remote;
    });
    for (const unit of removeUnits) {
        const metadata = unit.metadata as any;
        session.remoteAttrs.delete(metadata.dataSource.remote);
    }
    session.units = session?.units.filter((unit) => {
        const metadata = unit.metadata as any;
        return metadata.dataSource.remote !== dataSource.remote;
    });
    let remoteMaxTimeStamps = 0;
    session.remoteAttrs.forEach((attrs) => {
        remoteMaxTimeStamps = Math.max(<number>attrs.maxTimeStamp, remoteMaxTimeStamps);
    });
    session.endTimeAll = remoteMaxTimeStamps;
};
