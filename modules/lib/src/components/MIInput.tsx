/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import * as React from 'react';
import { type InputProps, type InputRef, type InputNumberProps, Input, InputNumber } from 'antd';
import { useTranslation } from 'react-i18next';
import styled from '@emotion/styled';

// 默认最大输入200个字符
const DEFAULT_MAX_LENGTH = 200;
// 默认输入数字范围为32位整数范围
const DEFAULT_MIN_INPUT_NUMBER = -2147483648;
const DEFAULT_MAX_INPUT_NUMBER = 2147483647;

const sizeOption = {
    small: 100,
    middle: 160,
    large: 240,
};

const StyledInput = styled(Input)`
    width: ${(props): number => (props.size && sizeOption[props.size]) ?? sizeOption.middle}px;
    height: ${(props): number | string => (props.height ?? 32)}px;
    background-color: ${(props): string => props.theme.bgColor};
    border-color: ${(props): string => props.theme.borderColorLighter};

    &:not(.ant-input-affix-wrapper-disabled):hover, &.ant-input-affix-wrapper-focused {
        border-color: ${(props): string => props.theme.primaryColor};
    }

    .ant-input {
        background-color: transparent;
        color: ${(props): string => props.theme.textColorPrimary};
        &::placeholder {
            color: ${(props): string => props.theme.textColorTertiary};
        }
    }

    .ant-input-clear-icon {
        color: ${(props): string => props.theme.textColorTertiary};
        &:hover {
            color: ${(props): string => props.theme.textColorPrimary};
        }
    }
`;

export const MIInput: React.FC<InputProps & React.RefAttributes<InputRef>> = (props) => {
    const { t } = useTranslation('lib');
    return (
        <StyledInput
            maxLength={DEFAULT_MAX_LENGTH}
            placeholder={t('Please enter')}
            {...props}
        />
    );
};

const StyledInputNumber = styled(InputNumber)`
    width: ${(props): number => (props.size && sizeOption[props.size]) ?? sizeOption.middle}px;
    height: ${(props): number | string => (props.height ?? 32)}px;
    background-color: ${(props): string => props.theme.bgColor};
    border-color: ${(props): string => props.theme.borderColorLighter};
    color: ${(props): string => props.theme.textColorPrimary};

    &:hover, &.ant-input-number-focused {
        border-color: ${(props): string => props.theme.primaryColor};
    }

    .ant-input-number-handler-wrap{
        background-color: transparent;
        .ant-input-number-handler {
            border-color: ${(props): string => props.theme.borderColorLighter};
            > span {
                color: ${(props): string => props.theme.textColorTertiary};
            }
            &:hover >span {
                color: ${(props): string => props.theme.primaryColor};
            }
            
            &.ant-input-number-handler-up-disabled, &.ant-input-number-handler-down-disabled {
                > span {
                    color: ${(props): string => props.theme.textColorTertiary};
                }
            }
        }
    }
`;

type ValueType = string | number;
export const MIInputNumber: ((props: InputNumberProps<ValueType> & {
    children?: React.ReactNode;
} & {
    ref?: React.Ref<HTMLInputElement>;
}) => React.ReactElement) & {
    displayName?: string;
} = (props) => {
    return (
        <StyledInputNumber
            min={DEFAULT_MIN_INPUT_NUMBER}
            max={DEFAULT_MAX_INPUT_NUMBER}
            maxLength={DEFAULT_MAX_LENGTH}
            formatter={(value): string => `${Number(value)}`}
            {...props}
        />
    );
};
