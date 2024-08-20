/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
export interface StringMap {
    [prop: string]: string;
}

export interface VoidFunction {
    (...rest: any[]): void;
}

export interface optionDataType {
    data?: any;
    key?: string;
    label: React.ReactNode;
    value: string | number;
}

export interface optionMapType {
    [props: string]: optionDataType[];
}
