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

import { KEYS } from '@insight/lib/utils';
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
