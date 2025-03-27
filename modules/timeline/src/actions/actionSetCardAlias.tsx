/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React from 'react';
import { register } from './register';
import type { Session } from '../entity/session';
import { CardUnit } from '../insight/units/AscendUnit';
import { Modal } from 'antd';
import { ThemeProvider } from '@emotion/react';
import { themeInstance } from 'ascend-theme';
import { SetAlias } from '../components/SetAlias';

const isSetAliasVisible = (session: Session): boolean => {
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit === undefined) {
        return false;
    }
    return selectedUnit instanceof CardUnit ?? false;
};

const setAlias = (session: Session): void => {
    Modal.confirm({
        modalRender: () => <ThemeProvider theme={themeInstance.getThemeType()}>
            <SetAlias session={session}></SetAlias>
        </ThemeProvider>,
        maskClosable: true,
        centered: true,
        width: 'auto',
    });
};

export const actionSetCardAlias = register({
    name: 'setCardAlias',
    label: 'timeline:contextMenu.Set alias',
    visible: (session) => isSetAliasVisible(session),
    perform: (session): void => {
        setAlias(session);
    },
});
