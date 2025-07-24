/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
interface TypeOption {
    label: string | number;
    value: string | number;
}
export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    deviceIds: any = {};
    threadIds: number[] = [];
    blockData: any = { blocks: [], minSize: 0, maxSize: 0, minTimestamp: 0, maxTimestamp: 0 };
    allocationData: any = { allocations: [], maxTimestamp: 0, minTimestamp: 0 };
    memoryData: any = { size: 0, name: '', subNodes: [] };
    funcData: any = { traces: [], maxTimestamp: 0, minTimestamp: 0 };
    memoryStamp: number = 0;
    deviceId: string = '';
    eventType: string = '';
    threadId: number | string = '';
    maxTime: number = 0;
    minTime: number = 0;
    deviceIdOpts: TypeOption[] = [];
    typeOpts: TypeOption[] = [];
    threadOps: TypeOption[] = [];
    legendSelect: any = {};
    synStartTime: number = 0;
    synEndTime: number = 0;
    searchFunc: string[] = [];
    funcOptions: any = [];
    constructor() {
        makeAutoObservable(this);
    }
}
