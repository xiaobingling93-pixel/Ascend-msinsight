/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import { makeAutoObservable } from 'mobx';

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    token = '';
    unitcount = 0;
    memoryRankIds: string[] = [];
    isWakeup: boolean = false;
    isCluster: boolean = false;
    isClusterMemoryCompletedSwitch: boolean = false;
    curRankIdsCount = 0;

    constructor() {
        makeAutoObservable(this);
    }
}
