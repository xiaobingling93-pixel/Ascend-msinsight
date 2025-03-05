/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import React from 'react';
import { Popover } from 'antd';
import styled from '@emotion/styled';
import type { PopoverProps } from 'antd/lib/popover';

export const MIPopover = styled((props: PopoverProps) => {
    const { children, ...restPops } = props;

    return <Popover {...restPops}>
        {props.children}
    </Popover>;
})`
    .ant-popover-inner-content {
        background-color:  ${(props): string => props.theme.contextMenuBgColor};
        padding: 4px 0;
        user-select: none;
    }

    .mi-popover-item {
        padding: 4px 16px;
        color: ${(props): string => props.theme.textColorPrimary};
        cursor: pointer;

        &:hover {
            color: #ffffff;
            background: ${(props): string => props.theme.primaryColorHover};
            transition: all .3s ease;
        }
    }
`;
