/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import * as React from 'react';
import { Pagination } from 'antd';
import type { PaginationProps } from 'antd/lib/pagination';
import styled from '@emotion/styled';

export const MIPagination = styled((props: PaginationProps) => <Pagination {...props} />)`
    color: ${(props): string => props.theme.textColorPrimary};
`;
