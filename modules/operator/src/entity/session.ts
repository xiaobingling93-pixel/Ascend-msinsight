/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { makeAutoObservable } from 'mobx';
export class Session {
    allRankIds: string[] = [];
    renderId: number = 0;
    total: number = 0;
    isDark: boolean = true;
    constructor() {
        makeAutoObservable(this);
    }
}
