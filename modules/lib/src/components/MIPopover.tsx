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
