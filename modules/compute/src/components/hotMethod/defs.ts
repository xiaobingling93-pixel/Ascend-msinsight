/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import { RegisterTrack } from '@/components/hotMethod/RegisterDependency';

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
const GPR_STATUS = 'GPR Status';
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
    [ASCENDC_INNER_CODE]: string;
    [ADDESS]: string;
    [SOURCE]: string;
    [PIPE]: string ;
    [REAL_STALL_CYCLES]: number;
    [THEORETICAL_STALL_CYCLES]: number;
    maxCycles: number;
    cycles: number;
    Cycles: number;
    [GPR_STATUS]: RegisterTrack[];
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
