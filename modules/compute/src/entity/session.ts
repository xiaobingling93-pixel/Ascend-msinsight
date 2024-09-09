/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
import type { JsonInstructionType } from '../components/hotMethod/defs';

export interface DirInfo {
    rankId: string;
    isCompare: boolean;
};
export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    coreList: string[] = [];
    sourceList: string[] = [];
    instructions: JsonInstructionType[] = [];
    parseStatus: boolean = false;
    updateId: number = 0;
    blockIdList: string[] = [];
    theme: string = 'dark';
    // global param
    dirInfo: DirInfo = { rankId: '', isCompare: false };
    constructor() {
        makeAutoObservable(this);
    }
}
