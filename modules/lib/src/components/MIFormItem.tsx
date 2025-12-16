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

import React, { type ReactNode, type CSSProperties } from 'react';
import styled from '@emotion/styled';

interface FormItemProps {
    label?: string;
    colon?: boolean;
    children: ReactNode;
    style?: CSSProperties;
    labelStyle?: CSSProperties;
    contentStyle?: CSSProperties;
}

const FormItem = styled.div<Partial<FormItemProps>>`
  display: flex;
  align-items: center;
  margin-bottom: 10px;
  .label {
    margin-right: 8px;
    font-size: ${(props): string => props.theme.fontSizeBase};
  }
`;
export const MIFormItem: React.FC<FormItemProps> = (props): JSX.Element => {
    const { label, colon = true, children, style, labelStyle, contentStyle } = props;

    return (
        <FormItem style={style}>
            <div className="label" style={labelStyle}>
                {label}{label && colon ? ':' : ''}
            </div>
            <div className="content" style={contentStyle}>{children}</div>
        </FormItem>
    );
};

export default MIFormItem;
