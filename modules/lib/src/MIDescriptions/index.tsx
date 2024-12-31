/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
        <Container style={style}>
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
