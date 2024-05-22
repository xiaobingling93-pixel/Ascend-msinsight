/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { makeAutoObservable } from 'mobx';
export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    allRankIds: string[] = [];
    renderId: number = 0;
    total: number = 0;
    isDark: boolean = true;
    parseCompleted: boolean = false;
    constructor() {
        makeAutoObservable(this);
    }
}
