/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
export interface TreeNodeType {
    id: string;
    projectName: string;
    label: string;
    cancelable?: boolean;
    children?: TreeNodeType[];
    origin?: TreeNodeType[];
};

export const indent = 1.2;
