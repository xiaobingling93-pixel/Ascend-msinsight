/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
    value?: string;
    x?: number;
    y?: number;
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
    value?: string;
}
export interface IdrawLine {
    points: string;
    id?: string;
    label?: string | number;
    labelXy?: Ixy;
    labelPosition?: orientType;
}
