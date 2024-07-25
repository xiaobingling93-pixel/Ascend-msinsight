/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import * as React from 'react';
import { Checkbox } from 'antd';
import styled from '@emotion/styled';
import type { CheckboxProps } from 'antd/lib/checkbox';

export const MICheckbox = styled((props: CheckboxProps & React.RefAttributes<HTMLInputElement>) => <Checkbox {...props} />)`
    color: ${(props): string => props.theme.textColorPrimary};
    font-size: 12px;
`;
