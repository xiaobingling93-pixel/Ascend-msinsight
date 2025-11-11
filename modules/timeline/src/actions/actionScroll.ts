/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
