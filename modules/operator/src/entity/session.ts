/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { makeAutoObservable } from 'mobx';

export interface DirInfo {
    rankId: string;
    isCompare: boolean;
};

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    allRankIds: string[] = [];
    renderId: number = 0;
    total: number = 0;
    isDark: boolean = true;
    parseCompleted: boolean = false;
    // global param
    dirInfo: DirInfo = { rankId: '', isCompare: false };
    constructor() {
        makeAutoObservable(this);
    }
}
