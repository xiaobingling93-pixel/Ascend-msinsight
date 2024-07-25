/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import * as React from 'react';
import { Button } from 'antd';
import styled from '@emotion/styled';
import type { ButtonProps } from 'antd/lib/button';

const sizeOption = {
    large: {
        width: 80,
        height: 32,
    },
    small: {
        width: 64,
        height: 24,
    },
};

export const MIButton = styled((props: ButtonProps) => <Button {...props} />)`
    height: ${(props): number => props.size === 'large' ? sizeOption.large.height : sizeOption.small.height}px;
    min-width: ${(props): number => props.size === 'large' ? sizeOption.large.width : sizeOption.small.width}px;
    padding: ${(props): string => props.size === 'large' ? '0 16px' : '0 8px'};

    > span {
        font-size: 12px;
        color: ${(props): string => props.theme.textColorSecondary};
    }

    &.ant-btn-default {
        background-color: transparent;
        border-color: ${(props): string => props.theme.borderColorLighter};
        &:hover {
            border-color: ${(props): string => props.theme.primaryColorHover};
            > span {
                color: ${(props): string => props.theme.primaryColorHover};
            }
        }
        &:disabled {
            background-color: ${(props): string => props.theme.bgColorDisabled};
            border-color: ${(props): string => props.theme.borderColorDisabled};
            > span {
                color: ${(props): string => props.theme.borderColorDisabled};
            }
        }
    }

    &.ant-btn-primary {
        > span {
            color: ${(props): string => props.theme.textColorFourth};
        }
        background-color: ${(props): string => props.theme.primaryColor};
        border-color: ${(props): string => props.theme.primaryColor};
        &:hover {
            background-color: ${(props): string => props.theme.primaryColorHover};
            border-color: ${(props): string => props.theme.primaryColorHover};
        }
        &:disabled {
            background-color: ${(props): string => props.theme.primaryColorDisabled};
            border-color: ${(props): string => props.theme.primaryColorDisabled};
            > span {
                color: ${(props): string => props.theme.textColorFourth};
                opacity: 0.6;
            }
        }
    }

    &.ant-btn-link {
        > span {
            color: ${(props): string => props.theme.primaryColorHover};
        }
    }
`;
