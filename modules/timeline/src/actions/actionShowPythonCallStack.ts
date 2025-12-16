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
import type { Session } from '../entity/session';
import type { ThreadMetaData } from '../entity/data';
import { runInAction } from 'mobx';

const showPythonCallStack = (session: Session): void => {
    // 必须只选中一个才能调用“显示、隐藏python调用栈”菜单项
    if (session.selectedUnits.length !== 1) {
        return;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit === undefined) {
        return;
    }
    const metadata = selectedUnit.metadata as ThreadMetaData;
    if (metadata.cardId !== undefined) {
        const pythonFunctionConfig = session.unitsConfig.filterConfig.pythonFunction as Record<string, boolean>;
        const isFilteredPythonFunction = pythonFunctionConfig?.[`${metadata.cardId}_${metadata.threadName}`] ?? false;
        runInAction(() => {
            session.unitsConfig.filterConfig.pythonFunction = { ...pythonFunctionConfig, [`${metadata.cardId}_${metadata.threadName}`]: !isFilteredPythonFunction };
            session.linkLines = {};
            session.singleLinkLine = {};
            session.renderTrigger = !session.renderTrigger;
        });
    }
};

const isMenuVisible = (session: Session): boolean => {
    // 必须只选中一个才能显示“显示、隐藏python调用栈”菜单项
    if (session.selectedUnits.length !== 1) {
        return false;
    }
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit === undefined) {
        return false;
    }
    return selectedUnit.havePythonFunction ?? false;
};

const isShowMenuVisible = (session: Session): boolean => {
    let isFilteredPythonFunction = true;
    const selectedUnit = session.selectedUnits[0];
    if (selectedUnit === undefined) {
        return false;
    }
    const metadata = selectedUnit.metadata as ThreadMetaData;
    if (metadata.cardId !== undefined) {
        isFilteredPythonFunction = (session.unitsConfig.filterConfig.pythonFunction as Record<string, boolean>)?.[`${metadata.cardId}_${metadata.threadName}`] ?? false;
    }

    return isFilteredPythonFunction;
};

export const actionShowPythonCallStack = register({
    name: 'showPythonCallStack',
    label: 'timeline:contextMenu.Show python call stack',
    visible: (session) => isMenuVisible(session) && isShowMenuVisible(session),
    perform: (session): void => {
        showPythonCallStack(session);
    },
});

export const actionHidePythonCallStack = register({
    name: 'hidePythonCallStack',
    label: 'timeline:contextMenu.Hide python call stack',
    visible: (session) => isMenuVisible(session) && !isShowMenuVisible(session),
    perform: (session): void => {
        showPythonCallStack(session);
    },
});
