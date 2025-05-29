/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { register } from './register';
import { runInAction } from 'mobx';

export const actionFitToScreen = register({
    name: 'fitToScreen',
    label: 'timeline:contextMenu.Fit to screen',
    visible: (session) => session.selectedData !== undefined,
    perform: (session): void => {
        runInAction(() => {
            if (session.selectedData !== undefined) {
                const selectedData = session.selectedData;
                session.domainRange = {
                    domainStart: selectedData.startTime,
                    domainEnd: selectedData.startTime + selectedData.duration,
                };
            }
        });
    },
});
