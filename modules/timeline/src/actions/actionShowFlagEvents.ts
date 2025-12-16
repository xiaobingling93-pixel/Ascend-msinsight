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

const hideOrShowFlagEvents = (session: Session): void => {
    if (!session.isSimulation) {
        return;
    }
    runInAction(() => {
        session.areFlagEventsHidden = !session.areFlagEventsHidden;
        if (session.areFlagEventsHidden) {
            session.singleLinkLine = {};
            session.linkLines = {};
            session.renderTrigger = !session.renderTrigger;
        }
    });
};

export const actionShowFlagEvents = register({
    name: 'showFlagEvents',
    label: 'timeline:contextMenu.Show flag events',
    visible: (session) => session.isSimulation && session.areFlagEventsHidden,
    perform: (session): void => {
        hideOrShowFlagEvents(session);
    },
});

export const actionHideFlagEvents = register({
    name: 'hideFlagEvents',
    label: 'timeline:contextMenu.Hide flag events',
    visible: (session) => session.isSimulation && !session.areFlagEventsHidden,
    perform: (session): void => {
        hideOrShowFlagEvents(session);
    },
});
