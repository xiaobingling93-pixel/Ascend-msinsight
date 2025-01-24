/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import styled from '@emotion/styled';
import { useTranslation } from 'react-i18next';
import { ImportDataIcon } from 'ascend-icon';
import { type Session } from '@/entity/session';
import { SessionAction } from '@/utils/enum';
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
    const { t } = useTranslation('framework');
    const [dialogOpen, setDialogOpen] = useState(false);
    const [currentProject, setCurrentProject] = useState('');

    const openDialog = (): void => {
        setDialogOpen(true);
    };
    const closeDialog = (): void => {
        setDialogOpen(false);
    };
    // 新导入数据
    const importData = (): void => {
        setCurrentProject('');
        openDialog();
    };

    // 在已有项目下导入
    useEffect(() => {
        if (session.actionListener.type === SessionAction.ADD_DATA_UNDER_PROJECT) {
            setCurrentProject(session.actionListener.value);
            openDialog();
        }
    }, [session.actionListener]);
    return <>
        <ImportContainer onClick={importData}>
            <ImportDataIcon/>
            <span>{t('Import Data')}</span>
        </ImportContainer>
        <FileExplorer
            currentProject={currentProject}
            dialogOpen={dialogOpen}
            closeDialog={closeDialog}
        />
    </>;
});

export default ImportData;
