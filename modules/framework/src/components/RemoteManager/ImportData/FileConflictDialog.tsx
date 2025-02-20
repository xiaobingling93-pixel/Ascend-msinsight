/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import { Button } from 'ascend-components';
import styled from '@emotion/styled';
import { Modal } from 'antd';
import { useTranslation } from 'react-i18next';
import { ProjectError } from '@/utils/enum';

const StyledModal = styled(Modal)`
    .ant-modal-footer {
        padding: 10px 0;
    }
`;

interface IProps {
    error: ProjectError;
    open: boolean;
    onConfrim: () => void;
    onCancel: () => void;
}

const TITLE_MAP = new Map([
    [ProjectError.FILE_NOT_EXIST, 'FileNotExist'],
    [ProjectError.PROJECT_NAME_CONFLICT, 'FileConflict'],
    [ProjectError.IS_UNSAFE_PATH, 'FileUnsafe'],
    [ProjectError.EXISTING_LARGE_FILES, 'LargeFile'],
    [ProjectError.EXCEEDS_MXIMUN_LENGTH, 'PathLengthExceeded'],
]);

const FileConflictDialog = ({ error, open, onConfrim, onCancel }: IProps): JSX.Element => {
    const { t } = useTranslation('framework');

    const title = (TITLE_MAP.get(error) ?? error) as string;
    const content = TITLE_MAP.get(error) !== undefined ? `${TITLE_MAP.get(error)}Content` : error;
    return <StyledModal
        maskClosable={false} title={t(title)} width={320} open={open} onCancel={onCancel}
        footer={<div>
            {[ProjectError.IS_UNSAFE_PATH, ProjectError.FILE_NOT_EXIST].includes(error)
                ? <></>
                : <Button onClick={onConfrim} type="primary" size="small" style={{ marginRight: 8 }} >{t('Confirm')}</Button>}
            <Button onClick={onCancel} size="small">{t('Cancel')}</Button>
        </div>}
    >
        {t(content as string)}
    </StyledModal>;
};

export default FileConflictDialog;
