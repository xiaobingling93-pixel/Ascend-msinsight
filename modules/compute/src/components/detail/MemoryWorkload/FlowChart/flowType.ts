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
export type Igraph = Inode[];

export type Inode = {
    name: string;
    container?: Irect[];
    rect?: Irect[];
    line?: Iline[];
} & InodePosition;
export interface InodePosition {
    x?: number;
    y?: number;
    left?: number | string;
    top?: number | string;
}
export type Irect = {
    name?: string;
    label?: string;
    labelXy?: Ixy;
    labels?: Ilabel[];
} & IrectPosition;

export interface IrectPosition {
    width: number;
    height: number;
    x?: number;
    y?: number;
    left?: number | string;
    top?: number | string;
}
export type Iline = {
    id?: string;
    name?: string;
    label?: string | number;
    labelXy?: {x: number;y: number};
    labelPosition?: 'top' | 'bottom' | 'left' | 'right';
} & IlinePosition;
export type orientType = 'left' | 'right' | 'top' | 'bottom';
export interface IlinePosition {
    points?: string;
    x1?: number;
    y1?: number;
    x2?: number;
    y2?: number;
    x?: number;
    y?: number;
    left?: number | string;
    top?: number | string;
    orient?: orientType;
    length?: number;
}
export interface Ixy {
    x: number;
    y: number;
}
export interface Ibox {
    x1: number;
    y1: number;
    x2: number;
    y2: number;
    width: number;
    height: number;
}
interface Ilabel {
    id?: string;
    name?: string;
    value?: string;
    x?: number;
    y?: number;
    top?: number | string;
    position?: orientType;
}
export type IdrawGraph = IdrawNode[];
export interface IdrawNode {
    x: number;
    y: number;
    container?: IdrawRect[];
    rect?: IdrawRect[];
    line?: IdrawLine[];
    name?: string;
}
export interface IdrawRect {
    name?: string;
    x: number;
    y: number;
    width: number;
    height: number;
    label?: string | number;
    labelXy?: Ixy;
    labels?: IdrawLabel[];
}
export interface IdrawLabel {
    x: number;
    y: number;
    id?: string;
    name?: string;
    value?: string;
    position?: orientType;
}
export interface IdrawLine {
    points: string;
    id?: string;
    name?: string;
    label?: string | number;
    labelXy?: Ixy;
    labelPosition?: orientType;
}
