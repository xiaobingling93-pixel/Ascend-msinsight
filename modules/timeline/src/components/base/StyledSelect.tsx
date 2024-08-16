/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import * as React from 'react';
import styled from '@emotion/styled';
import { Select } from 'antd';
import type { SelectProps } from 'antd/lib/select';
import { ReactComponent as PullDownIcon } from '../../assets/images/insights/PullDownIcon.svg';
import { StyledEmpty } from './StyledEmpty';

const StyledPullDownIcon = styled((props: React.SVGProps<SVGSVGElement>) => <PullDownIcon {...props}/>)`
    use {
        fill: ${(props): string => props.theme.devicePullDown};
    }
`;

/**
 *  for control the pulldown arrow style.
 *  because ant-select's open class name is on its root dom,
 *  cannot use CSS selector to choose the open select in itself directly.
 **/
const ArrowController = styled.div`
    .ant-select-open > .ant-select-arrow > svg {
        transform: rotate(180deg);
    }
    display: inline-block;
`;

export const StyledSelect = styled((props: SelectProps &
{ width: number; height?: number; backgroundColor?: string; itemPaddingLeft?: number; itemPaddingRight?: number }) =>
    <ArrowController>
        <Select
            suffixIcon={<StyledPullDownIcon/>}
            notFoundContent={<StyledEmpty/>}
            getPopupContainer={(trigger: HTMLElement): HTMLElement => trigger.parentNode as HTMLElement ?? React.Fragment}
            {...props}
        />
    </ArrowController>)`
        .ant-select-selector {
            border-radius: 4px !important;
            box-shadow: none !important;
            background-color: ${(props): string => props.backgroundColor ?? props.theme.bgColor} !important;
            padding: 0 11px 0 0 !important;

            .ant-select-selection-item {
                padding-left: ${(props): number => props.itemPaddingLeft ?? 15}px;
                padding-right: ${(props): number => props.itemPaddingRight ?? 0}px;
                line-height: ${(props): number => props.height ?? 32}px;
            }

            height: ${(props): number => props.height ?? 32}px !important;

            .ant-select-selection-placeholder {
                line-height: ${(props): number => props.height ?? 32}px;
            }
        }

        .ant-select-dropdown {
            background-color: ${(props): string => props.theme.bgColor};
            .ant-select-item-option-content {
                color: ${(props): string => props.theme.textColorPrimary};
            }
            width: unset !important;
            .ant-select-item-option-active:not(.ant-select-item-option-disabled) {
                background-color: ${(props): string => props.theme.tableRowSelect};
            }
            .ant-select-item-option-selected {
                background-color: transparent;
            }
        }

        input {
            height: ${(props): number => props.height ?? 32}px !important;
        }

        color: ${(props): string => props.theme.textColorPrimary};
        width: ${(props): number => props.width}px;
        height: ${(props): number => props.height ?? 32}px;
`;
