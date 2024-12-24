/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
`;
