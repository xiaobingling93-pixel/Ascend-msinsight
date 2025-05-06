/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import * as React from 'react';
import styled from '@emotion/styled';
import { Tabs } from 'antd';
import type { TabsProps } from 'antd/lib/tabs';

export const MITabs = styled((props: TabsProps) => <Tabs {...props}/>)`
    color: ${(props): string => props.theme.textColorPrimary};

    >.ant-tabs-nav .ant-tabs-tab,
    >div>.ant-tabs-nav .ant-tabs-tab {
        background-color: ${(props): string => props.theme.bgColorLight};
        border-color: ${(props): string => props.theme.borderColor};
    }

    &.ant-tabs-top>.ant-tabs-nav .ant-tabs-tab-active,
    &.ant-tabs-top>div>.ant-tabs-nav .ant-tabs-tab-active {
        background-color: ${(props): string => props.theme.bgColor};
        border-bottom-color: ${(props): string => props.theme.bgColor};
    }

    &.ant-tabs-bottom>.ant-tabs-nav:before,
    &.ant-tabs-bottom>div>.ant-tabs-nav:before,
    &.ant-tabs-top>.ant-tabs-nav:before,
    &.ant-tabs-top>div>.ant-tabs-nav:before {
        border-bottom-color: ${(props): string => props.theme.borderColor};
    }
`;
