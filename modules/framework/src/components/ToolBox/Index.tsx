/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import styled from '@emotion/styled';
import SwitchTheme from './SwitchTheme';
import SwitchLanguage from './SwitchLanguage';
import Version from './Version';

const Container = styled.div`
  position: absolute;
  right: 0;
  margin: 6px 20px 0 0;
  display: flex;
  align-items: center;
  > * {
    margin-left: 10px;
    cursor: pointer;
  }
  svg {
    width: 20px;
    height: 20px;
  }
`;

const Index = observer(() => {
    return <Container>
        <SwitchTheme/>
        <SwitchLanguage/>
        <Version/>
    </Container>;
});

export default Index;
