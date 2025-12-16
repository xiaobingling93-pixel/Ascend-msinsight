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

interface MIDescriptionsProps {
    title?: ReactNode;
    children: ReactNode;
    style?: CSSProperties;
}

interface MIDescriptionsItemProps {
    label: ReactNode;
    children?: ReactNode;
    style?: CSSProperties;
    value?: ReactNode;
}

const Container = styled.div<Partial<MIDescriptionsProps>>`
    display: flex;
    flex-wrap: wrap;
    gap: 40px;
`;

const Item = styled.div<Partial<MIDescriptionsItemProps>>`
    max-width: 400px;
    color: ${(props): string => props.theme.textColorPrimary};
    white-space: pre-wrap;
    .label{
        color: ${(props): string => props.theme.textColorTertiary};
        margin-bottom: 4px;
    }
    .content{
        word-break: break-all;
        font-size: 16px;
    }
`;

export const MIDescriptions: React.FC<MIDescriptionsProps> = (props): JSX.Element => {
    const { children, style } = props;

    return (
        <Container style={style} className="mi-descriptions">
            {children}
        </Container>
    );
};

export const MIDescriptionsItem: React.FC<MIDescriptionsItemProps> = (props): JSX.Element => {
    const { label, children } = props;

    return (
        <Item>
            <div className="label">{label}:</div>
            <div className="content">
                {children}
            </div>
        </Item>
    );
};

export default MIDescriptions;
