/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
export interface StringMap {
    [prop: string]: string;
}

export interface optionDataType {
    key?: string;
    label: React.ReactNode;
    value: string | number ;
}

export interface optionMapDataType {
    [props: string]: optionDataType[];
}

export interface CompareData<T> {
    compare: T;
    baseline: T;
    diff: T;
}
export interface KeydownInfo {
    hasCtrl: boolean;
    hasCommand: boolean;
    key: string;
    isMac: boolean;
}
