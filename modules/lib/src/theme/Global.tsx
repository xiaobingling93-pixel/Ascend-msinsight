/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { Global, css, useTheme, type Theme, type SerializedStyles } from '@emotion/react';
import React from 'react';

const antdRadioCss = (theme: Theme): SerializedStyles => css`
    .ant-radio-wrapper {
        color: ${theme.textColorPrimary};
        .ant-radio-inner {
            background-color: ${theme.bgColor};
            border-color: ${theme.borderColorLighter};
        }
    }
    .ant-radio-checked {
        .ant-radio-inner {
            border-color: ${theme.radioSelectedColor};
            &:after {
                background-color: ${theme.radioSelectedColor};
            }
        }
    }
`;

const antdTooltipCss = (theme: Theme): SerializedStyles => css`
    .ant-tooltip-arrow-content {
        --antd-arrow-background-color: ${theme.bgColorLight};
    }
    .ant-tooltip-inner {
        border-radius: 4px;
        background-color: ${theme.bgColorLight};
        border-color: ${theme.borderColorLight};
        color: ${theme.textColorPrimary};
        box-shadow: ${theme.boxShadow};
        white-space: 'pre-wrap';
        padding: 8px;
        font-size: 12px;
        max-width: 400px;
    }
`;

const formatterCss = css`
    .formatter{
        .row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 2px 0;

            .label{
                margin-right: 16px;
            }
            .value {
                font-weight: bold;
            }
        }
    }
`;

export const GlobalStyles = (): JSX.Element => {
    const theme = useTheme();
    return <Global
        styles={css`
            color: ${theme.textColorPrimary};
            .ant-select-dropdown {
                background-color: ${theme.bgColor};
                border: 1px solid ${theme.primaryColor};
            }
            .ant-select-item {
                color: ${theme.textColorPrimary};
                font-size: 12px;
            }
            .ant-select-item-option-selected:not(.ant-select-item-option-disabled) {
                color: ${theme.textColorPrimary};
                background-color: ${theme.primaryColor};
            }
            .ant-select-item-option-active:not(.ant-select-item-option-disabled) {
                background-color: ${theme.primaryColorHover};
            }
            .ant-form-item-label > label {
                color: ${theme.textColorPrimary};
            }
            .ant-tabs {
                color: ${theme.textColorSecondary};
            }
            .ant-tabs-tab.ant-tabs-tab-active .ant-tabs-tab-btn {
                color: ${theme.primaryColor};
            }
            ${antdRadioCss(theme)};
            .ant-empty-description{
                color: ${theme.textColorPrimary};
            }
            .ant-dropdown {
                background-color: ${theme.bgColorLight};
                box-shadow: ${theme.boxShadowDropDown};
            }
            .ant-table-filter-dropdown > div {
                background-color: ${theme.bgColorLight};
            }
            ${antdTooltipCss(theme)};
            ${formatterCss};
        `}
    />;
};
