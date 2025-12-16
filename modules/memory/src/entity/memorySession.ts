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
import { OperatorDetail, StaticOperatorListDetail } from './memory';
import type { CardRankInfo } from './session';
import { SortOrder } from 'antd/lib/table/interface';

export interface ConditionType<T, K> {
    options: T[];
    value: K;
}

export interface HostConditionType extends ConditionType<string, string> {
    cardsMap?: Map<string, CardRankInfo[]>;
}

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

export const DEFAULT_SHOW_WITHIN_INTERVAL = false;
export const DEFAULT_CARD_RANK_INFO: CardRankInfo = { rankInfo: { clusterId: '', host: '', rankName: '', rankId: '', deviceId: '' }, dbPath: '', index: 0 };

export const enum GroupBy {
    DEFAULT = 'Overall',
    STREAM = 'Stream',
    COMPONENT = 'Component',
};

export interface RangeFlagList {
    uid: string;
    timeStamp: number;
    timeDisplay: string;
    color: string;
    colorCache: string;
    description: string;
    descriptionCache: string;
    type: 'range';
    anotherTimeStamp: number;
};

export class MemorySession {
    // memory类型相关变量
    memoryType: string = MemoryGraphType.DYNAMIC;
    resourceType: string = DataResourceType.PYTORCH;

    // memory静态图变量
    memoryGraphIdList: string[] = [];
    memoryGraphId: string = '';

    // 顶部筛选条件相关变量
    hostCondition: HostConditionType = { options: [], value: '' };
    rankCondition: ConditionType<CardRankInfo & { value?: number }, number | undefined> = { options: [], value: undefined };
    groupId: GroupBy = GroupBy.DEFAULT;

    // 中部折线图框选和下方表格联动
    selectedRange?: SelectedRange;
    staticSelectedRange?: SelectedRange;

    // 底部表格筛选条件相关变量
    searchEventOperatorName: string = '';
    minSize: number = 0;
    maxSize: number = 0;
    defaultMinSize = 0;
    defaultMaxSize = 0;
    order: SortOrder = null;
    orderBy?: string;
    filters: { [key: string]: string } = {};
    rangeFilters: { [key: string]: [number, number] } = {};
    isBtnDisabled: boolean = true;
    // 是否仅查看在选中时间区间分配或释放内存的数据
    isOnlyShowAllocatedOrReleasedWithinInterval: boolean = DEFAULT_SHOW_WITHIN_INTERVAL;

    // 底部表格点选与上方折线图联动
    selectedRecord?: OperatorDetail;
    selectedStaticRecord?: StaticOperatorListDetail;

    // 底部表格分页相关变量
    current: number = 1;
    pageSize: number = 10;

    rangeFlagList: RangeFlagList[] = [];
    timelineOffset: number = 0;

    constructor() {
        makeAutoObservable(this);
    }

    getSelectedRankValue(): CardRankInfo {
        if (this.rankCondition.value === undefined) { return DEFAULT_CARD_RANK_INFO; }
        return this.rankCondition.value < this.rankCondition.options.length ? this.rankCondition.options[this.rankCondition.value] : DEFAULT_CARD_RANK_INFO;
    }

    get selectedRankId(): string {
        return this.getSelectedRankValue().rankInfo.rankId ?? '';
    }
};
