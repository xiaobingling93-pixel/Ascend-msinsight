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
