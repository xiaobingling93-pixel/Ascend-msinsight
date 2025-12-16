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
import { Tabs } from 'antd';
import type { TabsProps } from 'antd/lib/tabs';

export const MITabs = styled((props: TabsProps) => <Tabs {...props}/>)`
    color: ${(props): string => props.theme.textColorPrimary};

    >.ant-tabs-nav .ant-tabs-tab,
    >div>.ant-tabs-nav .ant-tabs-tab {
        background-color: ${(props) => props.type ==='card' ? props.theme.bgColorLight : 'transparent'};
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
