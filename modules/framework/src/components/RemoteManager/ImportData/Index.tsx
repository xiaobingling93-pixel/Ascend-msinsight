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
import SetBtn from './SetBtn';

const ImportContainer = styled.div`
    display: flex;
    align-items: center;
    padding: 8px;
    & > div:first-child {
        margin-right: 8px;
        flex: 1;
    }
    .btn-import > div:first-child{
        margin-right: 8px;
    }
`;

export const BtnItem = styled.div`
    display: flex;
    justify-content: center;
    padding: 8px;
    background: ${(props): string => props.theme.bgColorCommon};
    border-radius: var(--mi-border-radius-small);
    cursor: pointer;
    font-size: 12px;
    color: ${(props): string => props.theme.textColorPrimary};
    :not(.disabled):hover {
        color: ${(props): string => props.theme.primaryColor};
        transition: .3s;
    }

    &.disabled {
        cursor: not-allowed;
        pointer-events: none;
        color: ${(props): string => props.theme.textColorDisabled};
        &:hover {
            color: ${(props): string => props.theme.textColorDisabled};
        }
    }
    &.danger:hover {
        color: ${(props): string => props.theme.dangerColor};
    }
    &.small {
        padding: 4px;
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
        <ImportContainer>
            <BtnItem onClick={importData} className="btn-import"><ImportDataIcon/><span>{t('Import Data')}</span></BtnItem>
            <SetBtn session={session}/>
        </ImportContainer>
        <FileExplorer
            currentProject={currentProject}
            dialogOpen={dialogOpen}
            closeDialog={closeDialog}
        />
    </>;
});

export default ImportData;
