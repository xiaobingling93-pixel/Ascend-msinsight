/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { Session } from '@/entity/session';
import styled from '@emotion/styled';

const Container = styled.div` 
    position: absolute;
    right:0;
    color: ${(props): string => props.theme.textColorPrimary};
`;

interface IProps {
    session: Session;
}
const Index = observer(({ session }: IProps) => {
    return <Container>Tool Box </Container>;
});

export default Index;
