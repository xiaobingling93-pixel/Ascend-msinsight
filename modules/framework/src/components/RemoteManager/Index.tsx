/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { Session } from '@/entity/session';
import ImportData from './ImportData/Index';
import styled from '@emotion/styled';
import ProjectContents from './ProjectContents/Index';
import { DataManagerIcon } from '@insight/lib/icon';
import { useTranslation } from 'react-i18next';

const Container = styled.div`
    color:${(props): string => props.theme.textColorPrimary};
`;

const Header = styled.div`
    display: flex;
    justify-content: start;
    flex-direction: row;
    height: 36px;
    align-items: center;
    min-width: 80px;
    padding: 0 15px 0 5px;
    background: ${(props): string => props.theme.bgColorLight};
    border-bottom: 1px solid ${(props): string => props.theme.bgColorLight};
    font-size: 12px;
    line-height: 18px;
    font-weight: 400;
    user-select: none;
    > div {
        margin-right: 5px;
    }
`;

interface IProps {
    session: Session;
}

const Index = observer(({ session }: IProps) => {
    const { t } = useTranslation('framework');

    return <Container>
        <Header><DataManagerIcon/>{ t('Data Manager')}</Header>
        <ImportData session={session}/>
        <ProjectContents/>
    </Container>;
});

export default Index;
