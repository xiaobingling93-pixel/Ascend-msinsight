/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
import type { Ilinetable, JsonInstructionType } from '../components/hotMethod/defs';

export interface DirInfo {
    rankId: string;
    isCompare: boolean;
};

export enum InstructionSelectSource {
    DEFAULT = 'default',
    CACHE = 'cache',
}

export interface CacheUnit {
    cachelineId?: number;
    addressRange: string[][];
}
export const defaultCacheUnit = { cachelineId: -1, addressRange: [] };

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    // 代码行日志
    loggedCodeLines: Ilinetable[] = [];
    // 指令数据版本
    instrVersion: number = -1;
    coreList: string[] = [];
    sourceList: string[] = [];
    instructions: JsonInstructionType[] = [];
    parseStatus: boolean = false;
    updateId: number = 0;
    blockIdList: string[] = [];
    theme: string = 'dark';
    // global param
    dirInfo: DirInfo = { rankId: '', isCompare: false };
    computeAdvice?: string[];
    // 打开代码搜索工具栏
    openFind: boolean = false;
    // 指令高亮来源
    instructionSelectSource: InstructionSelectSource = InstructionSelectSource.DEFAULT;
    cacheUnit: CacheUnit = defaultCacheUnit;
    instructionUpdateId: number = 0;
    // 内存负载分析图-计算公式提示
    memoryIndicator: {name?: string;x: number;y: number} = { x: 0, y: 0 };
    constructor() {
        makeAutoObservable(this);
    }

    reset(): void {
        this.coreList = [];
        this.sourceList = [];
        this.parseStatus = false;
        this.blockIdList = [];
        this.instructions = [];
        // 指令表选中
        this.instructionSelectSource = InstructionSelectSource.DEFAULT;
        this.cacheUnit = defaultCacheUnit;
    }
}
