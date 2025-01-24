/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import styled from '@emotion/styled';
import { Modal, Typography } from 'antd';
import { useTranslation } from 'react-i18next';
import { Button, Input, Tooltip } from 'ascend-components';
import { RefreshIcon } from 'ascend-icon';
import { checkPathValid, getLastFilePath, getTrimedPath, getSearchDir } from '@/utils/Resource';
import { ProjectAction, ProjectError } from '@/utils/enum';
import { type DataSource, LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import type { CatalogActionListener, SearchResult } from './ResourceCatalog';
import ResourceCatalog, { CatalogAction } from './ResourceCatalog';
import { customConsole as console } from 'ascend-utils';
import { handleProjectAction } from '@/utils/Project';
const { Text } = Typography;

const FileExplorerContainer = styled.div`
    .project-name {
        display: flow;
        white-space: nowrap;
        text-overflow: ellipsis;
        overflow: hidden;
        font-weight: 700;
        padding-bottom: 10px;
    }
    // 输入框
    .ant-input-affix-wrapper {
        width: 100%;
    }
    .ant-input-show-count-suffix {
        color: ${(props): string => props.theme.infoColor};
    }
    .icon-refresh {
        cursor: pointer;
        color: ${(props): string => props.theme.textColorPlaceholder};
    }
    .icon-refresh:hover {
        color: ${(props): string => props.theme.primaryColor};
    }
    //提示文字
    .ant-typography{
        color: ${(props): string => props.theme.primaryColor};
    }
    .ant-typography-danger {
        color: ${(props): string => props.theme.dangerColor};
    }
`;

// 文件最大路径长度
const MAX_FILE_PATH_LENGTH = 260;
interface IProps {
    currentProject: string;
    dialogOpen: boolean;
    closeDialog: () => void;
}
// 文件资源管理器
const FileExplorer = observer(({ dialogOpen, closeDialog, currentProject }: IProps) => {
    const { t } = useTranslation('framework');
    const [inputPath, setInputPath] = useState(getLastFilePath());
    const [actionListener, setActionListener] = useState<CatalogActionListener>({ type: CatalogAction.NO_ACTION });
    const [hit, setHit] = useState<{alert: boolean;message: string;options?: Record<string, string | number>}>({ alert: false, message: 'FileSearchDescribe' });

    // 点击确认
    const handleConfirm = async(): Promise<void> => {
        const path = getTrimedPath(inputPath);
        setInputPath(path);
        // 若currentProject存在，在已有项目下导入数据，否则新增项目
        const projectName = currentProject !== '' ? currentProject : path;
        const dataSource: DataSource = { remote: LOCAL_HOST, port: PORT, projectName, dataPath: [path] };

        // 校验
        const validRes: ProjectError = await checkPathValid({ path, dataSource });
        // 校验通过
        if ([ProjectError.NO_ERRORS, ProjectError.IMPORTED].includes(validRes)) {
            const action = validRes === ProjectError.NO_ERRORS ? ProjectAction.ADD_FILE : ProjectAction.SWITCH_PROJECT;
            handleProjectAction({ action, dataSource, isConflict: false });
            // 校验告警
        } else {
            console.error(validRes);
        }
        closeDialog();
    };

    // 查询目录树
    const searchCatalog = (): void => {
        const path = getSearchDir(inputPath);
        setInputPath(path);
        setActionListener({ type: CatalogAction.SEARCH, value: path });
    };

    // 目录查询返回结果
    const handleSearchReturn = ({ success, result }: SearchResult): void => {
        setHit({ alert: !success, message: result?.message ? result.message : 'FileSearchDescribe', options: result?.options });
    };

    // 每次打开对话框，刷新目录
    useEffect(() => {
        if (dialogOpen) {
            searchCatalog();
        }
    }, [dialogOpen]);

    return <Modal maskClosable={false} title={t('File Explorer')} open={dialogOpen} onOk={closeDialog} onCancel={closeDialog}
        footer={<div>
            <Button onClick={handleConfirm} type="primary" style={{ marginRight: 8 }} >{t('Confirm')}</Button>
            <Button onClick={closeDialog}>{t('Cancel')}</Button>
        </div>}>
        <FileExplorerContainer>
            {!currentProject ? <></> : <span className="project-name">{t('Current Project')} ：{currentProject}</span>}
            <Input
                placeholder={t('FileSearchDescribe')}
                showCount
                maxLength={MAX_FILE_PATH_LENGTH}
                suffix={<Tooltip placement="bottom" title={t('RefreshDirectory')} ><RefreshIcon className={'icon-refresh'} onClick={searchCatalog}/></Tooltip>}
                value={inputPath}
                onChange={(e): void => setInputPath(e.target.value)}
                onPressEnter={searchCatalog}
            />
            <Text type={hit.alert ? 'danger' : undefined}>{t(hit.message, hit.options)}</Text>
            <ResourceCatalog
                actionListener={actionListener}
                onSelectedChange={(value: string): void => setInputPath(value)}
                onSearchReturnChange={handleSearchReturn}
            />
        </FileExplorerContainer>
    </Modal>;
});

export default FileExplorer;
