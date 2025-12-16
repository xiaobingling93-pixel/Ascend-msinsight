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
import { addNewFlag, addRangeFlag } from '../components/TimelineMarker';
import { getTimestamp } from '../utils/humanReadable';
import { runInAction } from 'mobx';

export const actionFlagMarkCreation = register({
    name: 'createFlagMark',
    label: '',
    once: true,
    perform: (session): void => {
        if (!session.showCreateFlagMarkKey) { return; }
        if (session.selectedRange !== undefined) { // flag range
            const [s, e] = [...session.selectedRange].sort((a, b) => a - b);
            const rangeStartTimeDisplay = getTimestamp(s, { precision: session.isNsMode ? 'ns' : 'ms' });
            runInAction(() => {
                session.timelineMaker.oldMarkedRange = session.selectedRange;
            });
            addRangeFlag(session, s, rangeStartTimeDisplay, e);
            return;
        }
        if (session.selectedData !== undefined) { // flag range select slice
            const selectedRange: [number, number] = [session.selectedData.startTime, session.selectedData.startTime + session.selectedData.duration];
            const rangeStartTimeDisplay = getTimestamp(selectedRange[0], { precision: session.isNsMode ? 'ns' : 'ms' });
            runInAction(() => {
                // 用来判断是否在同范围位置重复创建旗帜
                session.timelineMaker.oldMarkedRange = session.selectedRange;
            });
            addRangeFlag(session, selectedRange[0], rangeStartTimeDisplay, selectedRange[1], session.selectedData.name);
            return;
        }
        if (session.hoverMouseX !== null && session.scaleBag.timelineMarkerTimeScale) { // flag single
            const timeScale = session.scaleBag.timelineMarkerTimeScale;
            const timestamp = Math.floor(timeScale(session.hoverMouseX));
            const timeDisplay = getTimestamp(timestamp, { precision: session.isNsMode ? 'ns' : 'ms' });
            addNewFlag(session, timestamp, timeDisplay);
        }
    },
    keyTest: (event) => event.key.toLowerCase() === KEYS.K,
});
