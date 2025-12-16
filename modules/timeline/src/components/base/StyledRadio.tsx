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
import { Radio } from 'antd';
import type { RadioProps } from 'antd/lib/radio';

export const StyledRadio = styled((props: RadioProps & { fontSize?: number }) => <Radio {...props}/>)`
    color: ${(props): string => props.theme.fontColor};
    font-size: ${(props): number => typeof props.fontSize === 'undefined' ? 12 : props.fontSize}px;

    .ant-radio-checked > .ant-radio-inner {
        border-color: #1890ff;
        background-color: #1890ff;
    }

    .ant-radio-input:focus + .ant-radio-inner {
        box-shadow: unset;
    }

    .ant-radio-inner {
        &::after {
            background-color: #FFF;
        }
        background-color: ${(props): string => props.theme.deviceProcessBackgroundColor};
        border: 1px solid ${(props): string => props.theme.enclosureBorder};
    }
`;
