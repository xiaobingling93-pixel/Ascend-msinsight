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

import React from 'react';
import { register } from './register';
import type { Session } from '../entity/session';
import { CardUnit } from '../insight/units/AscendUnit';
import { Modal } from 'antd';
import { ThemeProvider } from '@emotion/react';
import { themeInstance } from '@insight/lib/theme';
import { SetAlias } from '../components/SetAlias';

const isSetAliasVisible = (session: Session): boolean => {
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit === undefined) {
        return false;
    }
    return (selectedUnit instanceof CardUnit && selectedUnit.metadata?.cardName !== 'Host') ?? false;
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
