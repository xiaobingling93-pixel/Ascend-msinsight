/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
const ADDESS = 'Address';
const SOURCE = 'Source';
const PIPE = 'Pipe';
const INSTRUCIONS_EXECUTED = 'Instructions Executed';
const CYCLES = 'Cycles';
export const LINE = 'Line';
const ADDRESS_RANGE = 'Address Range';
const REAL_STALL_CYCLES = 'RealStallCycles';
const THEORETICAL_STALL_CYCLES = 'TheoreticalStallCycles';
const REGISTER_NUM = 'RegisterNum';
export const ASCENDC_INNER_CODE = 'AscendC Inner Code';
export interface JsonInstructionType {
    [prop: string]: React.Key | React.Key[] | undefined;
    [ADDESS]: string ;
    [SOURCE]: string ;
    [PIPE]: string ;
    [INSTRUCIONS_EXECUTED]: number[] ;
    [CYCLES]: number[];
    [REAL_STALL_CYCLES]?: number[];
    [THEORETICAL_STALL_CYCLES]?: number[];
    [REGISTER_NUM]?: number[];
    [ASCENDC_INNER_CODE]: string;
};

export interface Iline {
    [prop: string]: React.Key | React.Key[] | string[][];
    [LINE]: number;
    [ADDRESS_RANGE]: string[][];
};

export interface Ilinetable {
    [LINE]: number;
    [ADDRESS_RANGE]?: string[][];
    cycles?: number;
    instructionsExecuted?: number;
};

export interface InstrsColumnType {
    [prop: string]: React.Key | React.Key[] ;
    [ASCENDC_INNER_CODE]: string;
    [ADDESS]: string ;
    [SOURCE]: string ;
    [PIPE]: string ;
    maxCycles: number;
    cycles: number;
};

export enum FieldType {
    SKIP = 0,
    INT = 1,
    FLOAT = 2,
    STRING = 3,
    PERCENTAGE = 4,
}

export const NOT_APPLICABLE = 'NA';

export enum InstructionVersion {
    DEFAULT = 90,
    'ASCENDC_INNER_CODE' = 0,
}

export interface Limit {
    maxSize: number;
    overlimit: boolean;
    current: number;
}
