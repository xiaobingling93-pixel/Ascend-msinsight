import { AutoComplete } from 'antd';
import React from "react";
import styled from "@emotion/styled";


export const StyledAutoComplete = styled(AutoComplete)`
    .ant-select-dropdown {
            border-radius: 16px;
            padding: 4px;
            background: transparent;
            box-shadow: none;
            .rc-virtual-list-holder-inner {
                & > div {
                    font-size: 1rem;
                    background: ${(props): string => props.theme.deviceProcessBackgroundColor};
                    color: ${(props): string => props.theme.filterColor};
                }
                .ant-select-item-option-active {
                    color: ${(props): string => props.theme.filterColor} !important;
                    background-color: ${(props): string => props.theme.flagListHoverColor} !important;
                    svg {
                        g {
                            fill: ${(props): string => props.theme.deviceProcessActiveFontColor};
                        }
                    }
                }
                .ant-select-item-option-selected .ant-select-item-option-state {
                    display: none;
                }
            }
            .rc-virtual-list-scrollbar-thumb {
                background-color: ${(props): string => props.theme.deviceProcessNotActiveFontColor} !important; /* 设置滚动条滑块的颜色为红色 */
            }
        }
`
