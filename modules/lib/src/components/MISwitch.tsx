/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
import { Switch, type SwitchProps } from 'antd';
import styled from '@emotion/styled';

export const MISwitch = styled((props: SwitchProps) => {
    return (
        <Switch {...props} />
    );
})`
    &.ant-switch-checked {
        background: ${(props): string => props.theme.borderColorLighter};
    }
    &:focus {
        box-shadow: none;
    }
`;
