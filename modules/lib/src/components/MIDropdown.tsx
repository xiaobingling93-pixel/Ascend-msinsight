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
import styled from '@emotion/styled';
import { Dropdown } from 'antd';
import type { DropdownButtonProps } from 'antd/lib/dropdown';
import { useTheme } from '@emotion/react';

const DropdownContainer = styled.div`
    .ant-dropdown-menu {
        transition: all .1s ease;
        user-select: none;
        min-width: 150px;
        padding: 0;
        border-radius: ${(props): string => props.theme.borderRadiusBase};
        background-color: ${(props): string => props.theme.contextMenuBgColor};
        box-shadow: ${(props): string => props.theme.boxShadowLight};

        .ant-dropdown-menu-item {
            grid-template-columns: 1fr 0.2fr;
            color: ${(props): string => props.theme.textColorPrimary};

            &:not(.ant-dropdown-menu-item-disabled):hover {
                background: ${(props): string => props.theme.primaryColorHover};
                color: #ffffff;
            }

            &.ant-dropdown-menu-item-disabled {
                color: ${(props): string => props.theme.textColorDisabled};
            }
            &.ant-dropdown-menu-item-disabled:hover {
                background: none;
            }
        }
    }
`;

export const MIDropdown: React.FC<DropdownButtonProps> = (props) => {
    const theme = useTheme();
    const { children, overlayStyle = {}, ...restProps } = props;
    return <Dropdown dropdownRender={(menu): React.ReactNode => <DropdownContainer>{menu}</DropdownContainer> }
        overlayStyle={{
            borderRadius: theme.borderRadiusBase,
            ...overlayStyle,
        }}
        {...restProps}>
        {children}
    </Dropdown>;
};
