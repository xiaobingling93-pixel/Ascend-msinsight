/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
import type { JsonInstructionType } from '../components/hotMethod/defs';
export class Session {
    coreList: string[] = [];
    sourceList: string[] = [];
    Instructions: JsonInstructionType[] = [];
    parseStatus: boolean = false;
    updateId: number = 0;
    constructor() {
        makeAutoObservable(this);
    }
}
