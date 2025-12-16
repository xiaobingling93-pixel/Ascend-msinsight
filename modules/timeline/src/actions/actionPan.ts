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
import { KEYS } from '@insight/lib/utils';
import type { Session } from '../entity/session';
import { runInAction } from 'mobx';
import { PAN_RATE } from '../entity/domain';
import { clamp } from 'lodash';

enum MoveDirection {
    LEFT = -1,
    RIGHT = 1,
}

const moveDomain = (session: Session, direction: number): void => {
    const { domainRange: { domainStart, domainEnd } } = session;
    const timeDuration = domainEnd - domainStart;
    const timeOffset = direction * PAN_RATE * timeDuration;
    const newEnd = clamp(domainEnd + timeOffset, timeDuration, session.endTimeAll ?? session.domain.defaultDuration);
    runInAction(() => {
        session.domainRange = { domainStart: newEnd - timeDuration, domainEnd: newEnd };
    });
};

export const actionPanLeft = register({
    name: 'panLeft',
    label: '',
    perform: (session): void => {
        moveDomain(session, MoveDirection.LEFT);
    },
    keyTest: (event) => event.key.toLowerCase() === KEYS.A || event.key === KEYS.ARROW_LEFT,
});

export const actionPanRight = register({
    name: 'panRight',
    label: '',
    perform: (session): void => {
        moveDomain(session, MoveDirection.RIGHT);
    },
    keyTest: (event) => event.key.toLowerCase() === KEYS.D || event.key === KEYS.ARROW_RIGHT,
});
