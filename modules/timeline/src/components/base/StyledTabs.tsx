/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import * as React from 'react';
import styled from '@emotion/styled';
import { Tabs } from 'antd';
import type { TabsProps } from 'antd/lib/tabs';

export const StyledTabs = styled((props: TabsProps & { fontSize?: number }) => <Tabs {...props}/>)`
    color: inherit;
`;
