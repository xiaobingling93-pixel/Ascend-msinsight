/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { Popconfirm, Tooltip, message } from 'antd';
import { DeleteIcon } from 'ascend-icon';
import { removeDataPath, removeProject } from '@/utils/Project';
import { useTranslation } from 'react-i18next';
import { openLoading } from '@/utils/useLoading';
import type { Session } from '@/entity/session';

interface IProps {
    isProject: boolean;
    projectIndex: number;
    dataPathIndex?: number;
    dataPath?: string;
    session?: Session;
    projectName?: string;
}
const DeleteConfirm = observer(({ isProject, projectIndex, dataPath, session, projectName }: IProps) => {
    const { t } = useTranslation('framework');
    let isSelectBaseline = false;
    const confirm = (): void => {
        if (session) {
            const { compareSet: { baseline, comparison } } = session;
            if (isProject) {
                isSelectBaseline = baseline.filePath.startsWith(projectName as string) || comparison.filePath.startsWith(projectName as string);
            } else {
                isSelectBaseline = baseline.filePath.startsWith(dataPath as string) || comparison.filePath.startsWith(dataPath as string);
            }
        }
        if (isSelectBaseline) {
            message.warning(t('BaselineDataComparisonDataCannotDeleted'));
        } else {
            openLoading();
            if (isProject) {
                removeProject(projectIndex);
            } else {
                if (dataPath !== undefined) {
                    removeDataPath(projectIndex, dataPath);
                }
            }
        }
    };
    return <Popconfirm placement="topLeft"
        title={isProject ? t('DeleteProjectConfirmDescribe') : t('DeleteItemConfirmDescribe')}
        onConfirm={confirm}
        okText={t('Yes')}
        cancelText={t('No')}
        destroyTooltipOnHide={{ keepParent: false }}
    >
        <Tooltip placement="top" title={isProject ? t('Delete All') : t('Delete Item')} destroyTooltipOnHide={{ keepParent: false }}>
            <DeleteIcon/>
        </Tooltip>
    </Popconfirm>;
});

export default DeleteConfirm;
