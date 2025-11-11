/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
        if (session.hoverMouseX !== null && session.scaleBag.timelineMarkerTimeScale) { // flag single
            const timeScale = session.scaleBag.timelineMarkerTimeScale;
            const timestamp = Math.floor(timeScale(session.hoverMouseX));
            const timeDisplay = getTimestamp(timestamp, { precision: session.isNsMode ? 'ns' : 'ms' });
            addNewFlag(session, timestamp, timeDisplay);
        }
    },
    keyTest: (event) => event.key.toLowerCase() === KEYS.K,
});
