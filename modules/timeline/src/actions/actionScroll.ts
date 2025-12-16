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
import { PINNED_UNIT_WRAPPER_SCROLLER_ID, UNIT_WRAPPER_SCROLLER_ID } from '../components/ChartContainer/Units/Units';

// 每次滚动的步长
const SCROLL_STEP = 20;
enum ScrollDirection {
    UP = -1,
    DOWN = 1,
}

const scrollWrapper = (session: Session, direction: ScrollDirection): void => {
    if (!session.scrollArea) {
        return;
    }
    const scrollElement = document.getElementById(session.scrollArea === 'pinned' ? PINNED_UNIT_WRAPPER_SCROLLER_ID : UNIT_WRAPPER_SCROLLER_ID);
    // 滚动方向，正数为向下滚动，反之向上
    const scrollDirection = direction;
    requestAnimationFrame(() => {
        scrollElement?.scrollBy(0, SCROLL_STEP * scrollDirection);
    });
};

export const actionScrollUp = register({
    name: 'scrollUp',
    label: '',
    perform: (session): void => {
        scrollWrapper(session, ScrollDirection.UP);
    },
    keyTest: (event) => event.key === KEYS.ARROW_UP,
});

export const actionScrollDown = register({
    name: 'scrollDown',
    label: '',
    perform: (session): void => {
        scrollWrapper(session, ScrollDirection.DOWN);
    },
    keyTest: (event) => event.key === KEYS.ARROW_DOWN,
});
