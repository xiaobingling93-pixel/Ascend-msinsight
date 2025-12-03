/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import { makeAutoObservable } from 'mobx';
import { RankInfo } from './memory';

export interface ICompareRankInfo {
    rankId: string;
    isCompare: boolean;
}

export interface CardInfo {
    cardId: string;
    dbPath: string;
    index: number;
}

export interface CardRankInfo {
    rankInfo: RankInfo;
    dbPath: string;
    index: number;
}

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    unitcount = 0;
    memoryCardInfos: CardRankInfo[] = [];
    isCluster: boolean = false;
    isAllMemoryCompletedSwitch: boolean = false;
    // rankId 实际是 cardId: `{host} {rankId}`
    compareRank: ICompareRankInfo = { rankId: '', isCompare: false };
    projectChangedTrigger: boolean = true; // 删除或切换工程后触发

    constructor() {
        makeAutoObservable(this);
    }
}
