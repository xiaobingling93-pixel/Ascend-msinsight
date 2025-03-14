/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import styled from '@emotion/styled';
import { Tooltip, type TreeDataNode } from 'antd';
import { Tree } from 'ascend-components';
import { HandleSingleDoubleClick } from 'ascend-utils';
import type { EventDataNode } from 'antd/lib/tree';
import { AddIcon, LocalImportIcon } from 'ascend-icon';
import { store } from '@/store';
import type { File, Session } from '@/entity/session';
import { type DataSource, LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import { ProjectAction, SessionAction } from '@/utils/enum';
import { loadHistoryProject, handleProjectAction } from '@/utils/Project';
import DeleteConfirm from './DeleteConfirm';
import { isSameFile } from './ContextMenu';
import { useTranslation } from 'react-i18next';
import EditableText from './EditableText';
import CheckMenu from './CheckMenu';
import { getRankId } from '@/utils/Rank';
import { cancelCompareData } from '@/utils/Compare';
import { closeLoading, openLoading } from '@/utils/useLoading';

const ContentsContainer = styled.div`
    margin-right: 10px;
    height: calc(100vh - 84px);
    overflow-y: auto;
    .ant-tree {
        padding: 0.4rem 0.8rem;
        background: none;
    }
    // 目录名
    .ant-tree-node-content-wrapper {
        display: inline-block;
        width: calc(100% - 48px);
        &.ant-tree-node-content-wrapper-open, &.ant-tree-node-content-wrapper-close {
            width: calc(100% - 24px);
        }
        &:hover {
            background: none;
        }
        &.ant-tree-node-selected {
            background: none;
        }
    }
    .ant-tree-title {
        display: inline-block;
        width: calc(100% - 24px);
        justify-content: space-between;
        flex: 1;
        color: ${(props): string => props.theme.textColorMenu};
    }
    .content-body {
        display: flex;
        align-items: center;
    }
    .content-name {
        display:block;
        width: 100%;
    }
    .content-text {
        overflow: hidden;
        white-space: nowrap;
        text-overflow: ellipsis;
        flex: 1;
    }
    // 二级目录
    .ant-tree-icon__docu + .ant-tree-title {
        width: 100%;
        font-size: 12px;
        color: ${(props): string => props.theme.textColorTertiary};
    }
    // 折叠图标
    .ant-tree-switcher-icon {
        color: ${(props): string => props.theme.icon};
    }
    // 右侧按钮
    .btn-box {
        align-items: center;
        justify-content: end;
        width: 40px;
        margin-left: 10px;
        display: none;
    }
    // 选中效果/鼠标滑动效果
    .ant-tree-treenode-selected, .ant-tree-treenode:hover {
        background: ${(props): string => props.theme.bgColorLight};

        .btn-box {
            display: flex;
        }
    }
    .ant-tree-treenode:hover {
        .content-name {
            width: calc(100% - 50px);
        }
        .ant-tree-checkbox + span .content-name {
            width: calc(100% - 70px);
        }
    }
    // 比对数据
    .baseline{
        font-weight:bold;
        color: ${(props): string => props.theme.primaryColor};
    }
    .comparison{
        font-weight:bold;
        color: ${(props): string => props.theme.warningColor};
    }
`;

const getNodeClass = (session: Session, file: File): string => {
    const { compareSet: { baseline, comparison } } = session;
    if (isSameFile(baseline, file)) {
        return 'baseline';
    } else if (isSameFile(comparison, file)) {
        return 'comparison';
    } else {
        return '';
    }
};

const handleRightClick = (file: File): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        session.selectedFile = file;
    });
};

// 导入数据按钮
function ImportDataBtn({ projectName, session }: {projectName: string;session: Session}): JSX.Element {
    const { t } = useTranslation('framework');
    return <Tooltip placement="bottom" title={t('Import Data')} destroyTooltipOnHide={{ keepParent: false }}>
        <AddIcon onClick={(): void => { session.actionListener = { type: SessionAction.ADD_DATA_UNDER_PROJECT, value: projectName }; }}/>
    </Tooltip>;
};

// 目录树数据
const getTreeData = (session: Session): TreeDataNode[] => {
    return session.dataSources.map((dataSource, dataSourceIndex) => ({
        key: dataSource.projectName,
        isLeaf: false,
        icon: <LocalImportIcon/>,
        title: <Tooltip mouseEnterDelay={0.3} placement="bottom" title={dataSource.projectName}>
            <span className={`content-body ${getNodeClass(session, { projectName: dataSource.projectName, filePath: '' })}`}>
                <span className="content-name" onContextMenu={(): void => handleRightClick({ projectName: dataSource.projectName, filePath: '' })}>
                    <EditableText text={dataSource.projectName}/></span>
                <div className="btn-box" onClick={(e): void => e.stopPropagation()}>
                    <ImportDataBtn projectName={dataSource.projectName} session={session}/>
                    <DeleteConfirm isProject={true} projectIndex={dataSourceIndex} />
                </div>
            </span>
        </Tooltip>,
        children: dataSource.dataPath?.map((path, dataPathIndex) => ({
            key: `${dataSource.projectName}-${path}`,
            isLeaf: true,
            title: <Tooltip mouseEnterDelay={0.3} placement="bottom" title={path}>
                <span className={`content-body ${getNodeClass(session, { projectName: dataSource.projectName, filePath: path })}`}>
                    <span className="content-text can-right-click"
                        onContextMenu={(): void => handleRightClick({ projectName: dataSource.projectName, filePath: path })}>
                        {getFilePathName({ projectName: dataSource.projectName, filePath: path })}
                    </span>
                    <div className="btn-box" onClick={(e): void => e.stopPropagation()}>
                        <DeleteConfirm isProject={false} projectIndex={dataSourceIndex} dataPathIndex={dataPathIndex}/>
                    </div>
                </span>
            </Tooltip>,
        })),
    }));
};

// 文件名
const getFilePathName = (file: File): string => {
    const rankId = getRankId(file);
    return `${rankId}${rankId === '' ? '' : ' : '}${file.filePath}`;
};

// 目录
const Contents = observer(({ session }: {session: Session}) => {
    const treeData = useMemo<TreeDataNode[]>(() => getTreeData(session), [session.dataSources, JSON.stringify(session.compareSet), session.rankMap]);

    // 展开情况: 默认展开所有工程，新导入工程默认展开
    const allProjectKeys = treeData.map(item => item.key);
    const [collapsedKeys, setCollapsedKeys] = useState(new Set());
    const expandedKeys = useMemo(() => allProjectKeys.filter(item => !collapsedKeys.has(item)), [collapsedKeys, allProjectKeys]);

    // 当前选中（打开）工程、文件
    const selectedKeys = useMemo(() => {
        if (session.activeDataSource.projectName !== '') {
            const { projectName, dataPath } = session.activeDataSource;
            return [projectName, `${projectName}-${dataPath[0]}`];
        }
        return [];
    }, [session.activeDataSource]);

    // 勾选目录
    const [checkedKeys, setCheckedKeys] = useState<React.Key[]>([]);
    const onCheck = (keys: any): void => { setCheckedKeys(keys); };
    const checkedProjectKeys = useMemo<React.Key[]>(() => checkedKeys.filter(key => allProjectKeys.includes(key)), [allProjectKeys, checkedKeys]);
    const isAllProjectChecked = checkedProjectKeys.length === allProjectKeys.length;
    // 全选、取消全选
    const toggleCheckAll = (checked: boolean): void => {
        setCheckedKeys(checked ? allProjectKeys : []);
    };

    // 点击目录
    const handleNodeClick = (keys: React.Key[], { node }: {node: EventDataNode<TreeDataNode>}): void => {
        const { activeDataSource, dataSources } = session;
        const [,projectIndex, dataPathIndex] = node.pos.split('-').map(index => Number(index));
        const dataSource: DataSource = dataSources[projectIndex];
        // 如果点击其它工程或者其它工程下文件
        if (dataSource.projectName !== activeDataSource.projectName) {
            handleProjectAction({ action: ProjectAction.SWITCH_PROJECT, dataSource, isConflict: false, selectedPath: dataSource.dataPath[dataPathIndex] });
        }
        // 如果点击的是文件
        if (node.isLeaf) {
            openLoading();
            runInAction(() => {
                session.activeDataSource = {
                    remote: LOCAL_HOST,
                    port: PORT,
                    projectName: dataSource.projectName,
                    dataPath: [dataSource.dataPath[dataPathIndex]],
                };
            });
            cancelCompareData();
            closeLoading();
        };
    };

    const handleSingleClick = (keys: React.Key[], nodeEvent: {node: EventDataNode<TreeDataNode>}): void => {
        // React不区分单击、双击
        // 如果点击项目名，为避免影响双击事件，增加了额外控制
        if (!nodeEvent.node.isLeaf) {
            HandleSingleDoubleClick.click(() => {
                handleNodeClick(keys, nodeEvent);
            }, 'projectName');
        } else {
            handleNodeClick(keys, nodeEvent);
        }
    };

    // 展开折叠
    const handleExpand = (keys: React.Key[], { node, expanded }: {expanded: boolean;node: TreeDataNode}): void => {
        if (expanded) {
            collapsedKeys.delete(node.key);
        } else {
            collapsedKeys.add(node.key);
        }
        setCollapsedKeys(new Set(collapsedKeys));
    };

    useEffect(() => {
        // 初始化，加载历史导入数据
        if (session.defaultConnected) {
            loadHistoryProject();
        }
    }, [session.defaultConnected]);

    useEffect(() => {
        // 进入编辑模式，选中全部
        if (session.projectContentEditStatus) {
            setCheckedKeys(allProjectKeys);
        }
    }, [session.projectContentEditStatus]);
    return <ContentsContainer>
        <CheckMenu editStatus={session.projectContentEditStatus} isAll={isAllProjectChecked} toggleCheckAll={toggleCheckAll} checkedKeys={checkedProjectKeys}/>
        <Tree
            checkable={session.projectContentEditStatus}
            checkedKeys={checkedKeys}
            onCheck={onCheck}
            blockNode={true}
            showIcon={true}
            treeData={treeData}
            selectedKeys={selectedKeys}
            multiple={true}
            expandedKeys={expandedKeys}
            onSelect={handleSingleClick}
            onExpand={handleExpand}
        />
    </ContentsContainer>
    ;
});

export default Contents;
