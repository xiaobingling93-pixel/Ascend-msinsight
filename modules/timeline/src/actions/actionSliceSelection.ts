/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { register } from './register';
import type { Session } from '../entity/session';
import { runInAction } from 'mobx';

const sliceSelectionVisible = (session: Session): boolean => {
    if (session.isTimeAnalysisMode) {
        return false;
    }
    return true;
};

const toggleSelectMode = (session: Session): void => {
    runInAction(() => {
        session.sliceSelection.active = !session.sliceSelection.active;
        session.renderTrigger = !session.renderTrigger;
        session.sliceSelection.startPos = [];
        session.sliceSelection.activeIsChanged = true;
    });
};

export const actionSliceSelection = register({
    name: 'sliceSelection',
    label: 'timeline:contextMenu.SliceSelectionMode',
    visible: (session: Session): boolean => sliceSelectionVisible(session),
    checked: (session) => session.sliceSelection.active,
    perform: (session): void => {
        toggleSelectMode(session);
    },
});
