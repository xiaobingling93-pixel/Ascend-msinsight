/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { CardRankInfo } from '../entity/session';

export interface StringMap {
    [prop: string]: string;
}

export interface VoidFunction {
    (...rest: any[]): void;
}

export interface OptionDataType {
    cards?: CardRankInfo[];
    key?: string;
    label: React.ReactNode;
    value: string | number;
}

export interface OptionMapType {
    rankOptions: Array<OptionDataType & { rankId: string; dbPath: string }>;
    hostOptions: OptionDataType[];
    groupOptions: OptionDataType[];
    topKOptions: OptionDataType[];
}
