/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { makeAutoObservable } from 'mobx';

export interface DirInfo {
    rankId: string;
    isCompare: boolean;
}

export interface CardInfo {
    cardId: string;
    dbPath: string;
    index: number;
}

export interface RankInfo {
    clusterId: string;
    host: string;
    rankName: string;
    rankId: string;
    deviceId: string;
}

export interface CardRankInfo {
    rankInfo: RankInfo;
    dbPath: string;
    index: number;
}

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    allCardInfos: CardRankInfo[] = [];
    renderId: number = 0;
    total: number = 0;
    isDark: boolean = true;
    // global param
    dirInfo: DirInfo = { rankId: '', isCompare: false };
    constructor() {
        makeAutoObservable(this);
    }
}
