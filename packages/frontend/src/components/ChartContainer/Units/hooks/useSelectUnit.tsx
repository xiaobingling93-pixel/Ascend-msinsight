import { runInAction } from 'mobx';
import { Session } from '../../../../entity/session';
import { getAutoKey } from '../../../../utils/dataAutoKey';
import { KeyedInsightUnit } from '../types';

type SelectUnit = (unit: KeyedInsightUnit) => void;
export const useSelectUnit = (session: Session): SelectUnit => {
    return (unit: KeyedInsightUnit): void => runInAction(() => {
        if (session.selectedUnits[0] === unit) return;
        session.selectedData = undefined;
        session.selectedUnitKeys = [getAutoKey(unit)];
        session.selectedUnits = [unit];
    });
};
