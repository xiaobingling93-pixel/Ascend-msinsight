/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { register } from './register';
import { KEYS } from '@insight/lib/utils';
import type { Session } from '../entity/session';
import { runInAction } from 'mobx';

const processMKeyEvent = (session: Session): void => {
    if (session.selectedRangeIsLock || session.isTimeAnalysisMode) {
        return;
    }

    let render = false;
    let range: number[] = [];
    if (!session.mKeyRender && session.selectedData === undefined) {
        // 页面无遮罩，无选中算子，点击m，页面无遮罩
        render = false;
        range = [];
    } else if (!session.mKeyRender && session.selectedData !== undefined) {
        // 页面无遮罩，选中算子，点击m，页面出现遮罩
        const selectedData = session.selectedData;
        range = [selectedData.startTime, selectedData.startTime + selectedData.duration];
        render = true;
    } else if (session.mKeyRender && session.selectedData === undefined) {
        // 页面有遮罩，无选中算子，点击m，页面无遮罩
        render = false;
        range = [];
    } else if (session.mKeyRender && session.selectedData !== undefined) {
        const selectedData = session.selectedData;
        const tempRange = [selectedData.startTime, selectedData.startTime + selectedData.duration];
        if (session.mMaskRange[0] === tempRange[0] && session.mMaskRange[1] === tempRange[1]) {
            // 页面有遮罩，有选中算子，但是同一个范围，点击m，页面无遮罩
            render = false;
            range = [];
        } else {
            // 页面有遮罩，有选中算子，但不是同一个范围，点击m，页面更换遮罩
            render = true;
            range = tempRange;
        }
    } else {
        render = session.mKeyRender;
        range = session.mMaskRange;
    }
    runInAction(() => {
        session.mKeyRender = render;
        session.mMaskRange = range;
    });
};

// M 键高亮选中算子区域
export const actionToggleSelection = register({
    name: 'toggleSelection',
    label: '',
    once: true,
    perform: (session): void => {
        processMKeyEvent(session);
    },
    keyTest: (event) => event.key.toLowerCase() === KEYS.M,
});
