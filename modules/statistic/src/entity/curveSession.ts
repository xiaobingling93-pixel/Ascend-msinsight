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

import { makeAutoObservable } from 'mobx';
import { DataDetail } from './curve';

export interface ConditionType {
    options: string[];
    value: string;
    ranks?: Map<string, string[]>;
};

export interface SelectedRange {
    startTs: string;
    endTs: string;
};

export class CurveSession {
    // 顶部筛选条件相关变量
    rankIdCondition: ConditionType = { options: [], value: '' };
    groupCondition: Array<{ label: string; value: string }> = [];
    groupId: string = '';
    // 中部折线图框选和下方表格联动
    selectedRange?: SelectedRange;
    isBtnDisabled: boolean = true;
    selectedRecord?: DataDetail;
    // 底部表格分页相关变量
    current: number = 1;
    pageSize: number = 10;

    constructor() {
        makeAutoObservable(this);
    }
}
