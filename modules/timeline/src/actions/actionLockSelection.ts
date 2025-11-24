/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { register } from './register';
import { KEYS } from '@insight/lib/utils';
import type { Session } from '../entity/session';
import { runInAction } from 'mobx';

const lockSelectionMenuVisible = (session: Session): boolean => {
    if (session.selectedRangeIsLock || session.isTimeAnalysisMode) {
        return false;
    }
    if (session.selectedRange === undefined || session.selectedUnits === undefined) {
        return false;
    }
    return session.selectedUnits.length !== 0;
};

const lockSelectionArea = (session: Session): void => {
    if (session.selectedRange === undefined || session.selectedUnits === undefined) {
        return;
    }
    if (session.selectedUnits.length === 0) {
        return;
    }
    runInAction(() => {
        session.selectedRangeIsLock = true;
        session.lockUnitCount = session.selectedUnits.length;
        session.lockRange = session.selectedRange;
        session.lockUnit = session.selectedUnits;
    });
};

const unlockSelectionArea = (session: Session): void => {
    runInAction(() => {
        session.selectedRangeIsLock = false;
        session.lockUnitCount = 0;
        session.lockRange = undefined;
        session.lockUnit = [];
    });
};

export const actionLockSelection = register({
    name: 'lockSelection',
    label: 'timeline:contextMenu.Lock selection area',
    visible: (session: Session): boolean => lockSelectionMenuVisible(session),
    perform: (session): void => {
        lockSelectionArea(session);
    },
    keyTest: (event) => event.key.toLowerCase() === KEYS.S,
});

export const actionUnLockSelection = register({
    name: 'unlockSelection',
    label: 'timeline:contextMenu.Unlock selection area',
    visible: (session: Session): boolean => session.selectedRangeIsLock,
    perform: (session): void => {
        unlockSelectionArea(session);
    },
    keyTest: (event) => event.key.toLowerCase() === KEYS.S,
});
