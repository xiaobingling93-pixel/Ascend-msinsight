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
                    background: ${props => props.theme.deviceProcessBackgroundColor};
                    color: ${props => props.theme.filterColor};
                }
                .ant-select-item-option-active {
                    color: ${props => props.theme.filterColor} !important;
                    background-color: ${props => props.theme.flagListHoverColor} !important;
                    svg {
                        g {
                            fill: ${props => props.theme.deviceProcessActiveFontColor};
                        }
                    }
                }
                .ant-select-item-option-selected .ant-select-item-option-state {
                    display: none;
                }
            }
        }
`
