/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { Global, css, useTheme } from '@emotion/react';
import React from 'react';
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
            .ant-radio-wrapper {
                color: ${theme.textColorPrimary};
            }
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
        `}
    />;
};
