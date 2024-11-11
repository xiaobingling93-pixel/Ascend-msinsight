/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState } from 'react';
import { observer } from 'mobx-react';
import { Session } from '@/entity/session';
import styled from '@emotion/styled';
import { useTranslation } from 'react-i18next';
import { ImportDataIcon } from 'ascend-icon';
import FileExplorer from './FileExplorer';

const ImportContainer = styled.div`
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 8px;
    margin: 8px;
    background: ${(props): string => props.theme.bgColorCommon};
    border-radius: var(--mi-border-radius-small);
    cursor: pointer;
    font-size: 12px;
    color:${(props): string => props.theme.textColorPrimary};
    :hover{
        color: ${(props): string => props.theme.primaryColor};
        transition: .3s;
    }
    &>div:first-child{
        margin-right: 8px;
    }
`;

const ImportData = observer(({ session }: {session: Session}) => {
    const [dialogOpen, setDialogOpen] = useState(false);
    const { t } = useTranslation('framework');

    const handleOk = (): void => {
        setDialogOpen(false);
    };

    const handleCancel = (): void => {
        setDialogOpen(false);
    };

    return <>
        <ImportContainer onClick={(): void => { setDialogOpen(true); }}>
            <ImportDataIcon/>
            <span>{t('Import Data')}</span>
        </ImportContainer>
        <FileExplorer dialogOpen={dialogOpen} handleOk={handleOk} handleCancel={handleCancel} session={session}/>
    </>;
});

export default ImportData;
