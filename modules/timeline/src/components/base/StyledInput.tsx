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
import { Input } from 'antd';
import type { InputProps, InputRef } from 'antd/lib/input';

const Support = React.forwardRef(
    (props: InputProps & { minwidth: number; isshow: number; width?: number }, ref: React.ForwardedRef<InputRef>) => {
        return <Input {...props} ref={ref} />;
    },
);
Support.displayName = 'Support';
export const StyledInput = styled(Support)`
    box-sizing: border-box;
    border-radius: 20px;
    min-width: ${(props): number => props.minwidth}px;
    width: ${(props): number => props.width as number}px;
    height: ${(props): number => props.height as number}px;
    border: 1px solid  ${(props): string => props.theme.enclosureBorder};
    background-color: transparent;
    padding: 0 8px;
    margin: 0 5px;
    display: ${(props): string => props.isshow === 1 ? 'inline-flex' : 'none'};
    align-items: center;
    color: ${(props): string => props.theme.fontColor};

    .ant-input {
        background-color: transparent;
        font-size: 14px;
        caret-color: ${(props): string => props.theme.searchInputCaretColor};
        color: ${(props): string => props.theme.fontColor};
        padding: 0;
        height: 100%;
        border: none;
        outline: none;
        box-shadow: none;
        &:focus {
            box-shadow: none;
        }
    }

    .ant-input-suffix {
        height: 100%;
        color: ${(props): string => props.theme.svgBackgroundColor};
        font-size: 12px;
        display: flex;
        align-items: center;
        margin-right: -4px;

        .ant-input-clear-icon, .anticon.ant-input-clear-icon {
            color: ${(props): string => props.theme.textColorTertiary};
        }
    }

    input[type=number]::-webkit-outer-spin-button,
    input[type=number]::-webkit-inner-spin-button {
        -webkit-appearance: none;
        appearance: none;
        margin: 0;
    }
`;
