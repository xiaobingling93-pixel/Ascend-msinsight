/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { register } from './register';
import { KEYS } from '@insight/lib/utils';
import { runInAction } from 'mobx';

export const actionToggleBottomPanel = register({
    name: 'toggleBottomPanel',
    label: '',
    perform: (session): void => {
        runInAction(() => {
            session.showBottomPanel = !session.showBottomPanel;
        });
    },
    once: true,
    keyTest: (event) => event.key.toLowerCase() === KEYS.Q,
});
