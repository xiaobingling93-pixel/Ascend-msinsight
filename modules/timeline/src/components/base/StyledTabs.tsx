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

export const StyledTabs = styled((props: TabsProps & { fontSize?: number }) => <Tabs {...props}/>)`
    color: ${(props): string => props.theme.textColorPrimary};
    &.ant-tabs-top {
        > .ant-tabs-nav {
            padding: 0 20px;
            margin: 0;
        }
    }
    .ant-tabs-tab {
        padding: 0 2px;
    }
    .ant-tabs-content {
        height: 100%;
    }
    .ant-tabs-tabpane {
        height: 100%;
    }
    .ant-tabs-content-holder {
        height: 100%;
    }
`;
