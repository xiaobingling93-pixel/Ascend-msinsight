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
import * as React from 'react';
import styled from '@emotion/styled';
import { Checkbox } from 'antd';
import type { CheckboxProps } from 'antd/lib/checkbox';

export const StyledCheckbox = styled((props: CheckboxProps & { fontSize?: number }) => <Checkbox {...props}/>)`
    & > span {
        color: ${(props): string => props.disabled ? props.theme.disabledFontColor : props.theme.fontColor} !important;
        font-size: ${(props): number => props.fontSize === undefined ? 12 : props.fontSize}px;
    }

    .ant-checkbox-checked > .ant-checkbox-inner {
        border-color: ${(props): string => props.disabled ? 'transparent' : '#1890ff'} !important;
        background-color: ${(props): string => props.disabled ? props.theme.templateBackgroundColor : '#1890ff'};
    }

    .ant-checkbox-inner {
        &:focus {
            box-shadow: unset;
        }
        background-color: ${(props): string => props.theme.deviceProcessBackgroundColor};
        border: 1px solid ${(props): string => props.theme.enclosureBorder};
    }
`;
