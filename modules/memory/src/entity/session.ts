/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import { makeAutoObservable } from 'mobx';

export interface ICompareRankInfo {
    rankId: string;
    isCompare: boolean;
}

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    unitcount = 0;
    memoryRankIds: string[] = [];
    isCluster: boolean = false;
    shouldRefresh: boolean = true;
    isClusterMemoryCompletedSwitch: boolean = false;
    compareRank: ICompareRankInfo = { rankId: '', isCompare: false };

    constructor() {
        makeAutoObservable(this);
    }
}
