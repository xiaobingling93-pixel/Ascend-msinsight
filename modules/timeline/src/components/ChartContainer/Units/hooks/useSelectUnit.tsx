/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import { runInAction } from 'mobx';
import type { Session } from '../../../../entity/session';
import { getAutoKey } from '../../../../utils/dataAutoKey';
import type { KeyedInsightUnit } from '../types';

type SelectUnit = (unit: KeyedInsightUnit) => void;
export const useSelectUnit = (session: Session): SelectUnit => {
    return (unit: KeyedInsightUnit): void => runInAction(() => {
        session.selectedData = undefined;
        session.selectedUnits = [unit];
    });
};

export const useSelectUnits = (session: Session): SelectUnit => {
    return (unit: KeyedInsightUnit): void => runInAction(() => {
        if (session.selectedUnits.includes(unit)) {
            return;
        }
        if (session.selectedRangeIsLock) {
            return;
        }
        session.selectedUnitKeys = [...session.selectedUnitKeys, getAutoKey(unit)];
        session.selectedUnits.push(unit);
    });
};

export const useDeselectUnits = (session: Session): SelectUnit => {
    return (unit: KeyedInsightUnit): void => runInAction(() => {
        if (session.selectedRangeIsLock) {
            return;
        }
        const unitKey = getAutoKey(unit);
        const unitIndex = session.selectedUnits.findIndex(selectedUnit => getAutoKey(selectedUnit) === unitKey);

        if (unitIndex !== -1) {
            session.selectedUnits.splice(unitIndex, 1);
            session.selectedUnitKeys = session.selectedUnitKeys.filter((key) => key !== unitKey);
        }
    });
};
