/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
const ADDESS = 'Address';
const SOURCE = 'Source';
const PIPE = 'Pipe';
const INSTRUCIONS_EXECUTED = 'Instructions Executed';
const INSTRUCION_EXECUTED = 'Instruction Executed';
const CYCLES = 'Cycles';
const CYCLE = 'Cycle';
const LINE = 'Line';
const ADDRESS_RANGE = 'Address Range';
export interface JsonInstructionType {
    [ADDESS]: string ;
    [SOURCE]: string ;
    [PIPE]: string ;
    [INSTRUCIONS_EXECUTED]: number[] ;
    [CYCLES]: number[];
};

export interface Iline {
    [LINE]: number;
    [CYCLE]: number;
    [INSTRUCION_EXECUTED]: number;
    [ADDRESS_RANGE]: string[][];
};

export interface Ilinetable {
    [LINE]: number;
    [ADDRESS_RANGE]?: string[][];
    cycles?: number;
    instructionsExecuted?: number;
};

export interface InstrsColumnType {
    [ADDESS]: string ;
    [SOURCE]: string ;
    [PIPE]: string ;
    maxCycles: number;
    cycles: number;
    instructionsExecuted: number;
};
