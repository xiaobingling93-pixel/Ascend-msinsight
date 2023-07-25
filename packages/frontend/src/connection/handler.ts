import { store } from '../store';
import { CardMetaData } from '../entity/data';
import { runInAction } from 'mobx';
import { handleMap, recursiveExpandUnit } from '../insight/units/unitFunc';
import { processUnits } from '../entity/insight';

export const parseSuccessHandler = (data: any): void => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    runInAction(() => {
        if (!session) {
            return;
        }
        session.units.forEach((unit) => {
            if ((unit.metadata as CardMetaData).cardId === data.unit.metadata.cardId) {
                handleMap(data.unit);
                recursiveExpandUnit(data.unit.children ?? [], unit);
            }
        });
        session.startRecordTime = 0;
        session.endTimeAll = data.maxTimeStamp;
        processUnits(session.units, 'download');
        if (data.startTimeUpdated === true) {
            session.simpleCache.clear();
        }
    });
};
