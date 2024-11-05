/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { makeAutoObservable } from 'mobx';
import { OperatorDetail, StaticOperatorListDetail } from './memory';

export interface ConditionType {
    options: string[];
    value: string;
    ranks?: Map<string, string[]>;
};

export const enum MemoryGraphType {
    DYNAMIC = 'dynamic',
    STATIC = 'static',
};

export const enum DataResourceType {
    PYTORCH = 'Pytorch',
    MINDSPORE = 'MindSpore',
};

export interface SelectedRange {
    startTs: number;
    endTs: number;
};

export const DEFAULT_SIZE_CONDITION = 1000000;

export class MemorySession {
    // memory类型相关变量
    memoryType: string = MemoryGraphType.DYNAMIC;
    resourceType: string = DataResourceType.PYTORCH;

    // memory静态图变量
    memoryGraphIdList: string[] = [];
    memoryGraphId: string = '';

    // 顶部筛选条件相关变量
    hostCondition: ConditionType = { options: [], value: '' };
    rankIdCondition: ConditionType = { options: [], value: '' };
    groupId: string = 'Overall';

    // 中部折线图框选和下方表格联动
    selectedRange?: SelectedRange;
    staticSelectedRange?: SelectedRange;

    // 底部表格筛选条件相关变量
    searchEventOperatorName: string = '';
    minSize: number = 0;
    maxSize: number = DEFAULT_SIZE_CONDITION;
    isBtnDisabled: boolean = true;

    // 底部表格点选与上方折线图联动
    selectedRecord?: OperatorDetail;
    selectedStaticRecord?: StaticOperatorListDetail;

    // 底部表格分页相关变量
    current: number = 1;
    pageSize: number = 10;

    constructor() {
        makeAutoObservable(this);
    }
};
