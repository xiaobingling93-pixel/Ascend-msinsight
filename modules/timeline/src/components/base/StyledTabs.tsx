/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
