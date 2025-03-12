/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { KEYS } from 'ascend-utils';
import { register } from './register';
import { runInAction } from 'mobx';
import type { Session } from '../entity/session';

function zoomIntoSelection(session: Session): void {
    runInAction(() => {
        if (session.selectedRange !== undefined) {
            session.domainRange = { domainStart: session.selectedRange[0], domainEnd: session.selectedRange[1] };
        }
    });
}

export const actionZoomIntoSelection = register({
    name: 'zoomIntoSelection',
    label: 'timeline:contextMenu.Zoom into selection',
    disabled: (session) => session.selectedData?.duration === 0,
    visible: (session) => session.selectedRange !== undefined,
    perform: (session): void => {
        zoomIntoSelection(session);
    },
    keyTest: (event) => {
        return event.shiftKey && event.key.toLowerCase() === KEYS.Z;
    },
});
