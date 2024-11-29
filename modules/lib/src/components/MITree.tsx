/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
import { Tree, type TreeProps } from 'antd';
import type RcTree from 'rc-tree';
import styled from '@emotion/styled'; import type { BasicDataNode } from 'rc-tree';
import type { DataNode } from 'rc-tree/lib/interface';

export const MITree = styled(<T extends BasicDataNode | DataNode = DataNode>(props: React.PropsWithChildren<TreeProps<T>> & {
    ref?: React.Ref<RcTree>;
}) => {
    return (
        <Tree {...props} />
    );
})`
    color: ${(props): string => props.theme.textColorPrimary};
    background: ${(props): string => props.theme.bgColorDark};
    font-size: 14px;
`;
