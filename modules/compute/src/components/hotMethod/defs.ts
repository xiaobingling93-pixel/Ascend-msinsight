/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

export interface JsonFileType {
    Source: string;
    Lines: JsonFileLineType[];
};

export interface JsonFileLineType {
    Line: number;
    Cycles: number[];
    'Instructions Executed': number[];
    'Address Range': string[][];
};

export interface JsonInstructionType {
    Address: string ;
    Source: string ;
    Pipe: string ;
    'Instructions Executed': number[] ;
    Cycles: number[];
};

export interface CodeLineType {
    Line: number;
    Cycles?: number;
    'Instructions Executed'?: number;
    'Address Range'?: string[][];
};

export interface InstrsColumnType {
    Address: string ;
    Source: string ;
    Pipe: string ;
    'Instructions Executed': number;
    Cycles: number;
    maxCycles: number;
};
