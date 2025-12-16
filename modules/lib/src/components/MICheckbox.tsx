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
import { Checkbox } from 'antd';
import styled from '@emotion/styled';
import type { CheckboxGroupProps, CheckboxProps } from 'antd/lib/checkbox';

export const MICheckbox = styled((props: CheckboxProps & React.RefAttributes<HTMLInputElement>) => <Checkbox {...props} />)`
    color: ${(props): string => props.theme.textColorPrimary};

    .ant-checkbox-inner {
        background-color: ${(props): string => props.theme.bgColor};
        border-color: ${(props): string => props.theme.borderColorLighter};
    }

    &.ant-checkbox-wrapper-checked .ant-checkbox-inner {
        background-color: ${(props): string => props.theme.primaryColor};
        border-color: ${(props): string => props.theme.primaryColor};
    }
`;

export const MICheckboxGroup = styled((props: CheckboxGroupProps & React.RefAttributes<HTMLInputElement>) => <Checkbox.Group {...props} />)`
    .ant-checkbox-wrapper {
        color: ${(props): string => props.theme.textColorPrimary};
    }

    .ant-checkbox-inner {
        background-color: ${(props): string => props.theme.bgColor};
        border-color: ${(props): string => props.theme.borderColorLighter};
    }

    .ant-checkbox-wrapper-checked .ant-checkbox-inner {
        background-color: ${(props): string => props.theme.primaryColor};
        border-color: ${(props): string => props.theme.primaryColor};
    }

    .ant-checkbox-disabled {
        + span {
            color: ${(props): string => props.theme.textColorDisabled};
        }
        .ant-checkbox-inner {
            border-color: ${(props): string => props.theme.borderColorDisabled} !important;
        }
    }
`;
