/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import styled from '@emotion/styled';
import { type TreeDataNode } from 'antd';
import { Tree } from 'ascend-components';
import type { EventDataNode } from 'antd/lib/tree';
import { AddIcon, LocalImportIcon } from 'ascend-icon';
import { store } from '@/store';
import type { File, Session } from '@/entity/session';
import { type DataSource, LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import { ProjectAction, SessionAction } from '@/utils/enum';
import { loadHistoryProject, handleProjectAction } from '@/utils/Project';
import DeleteConfirm from './DeleteConfirm';
import { isSameFile } from './ContextMenu';

const ContentsContainer = styled.div`
    padding: 0.5rem 0.8rem;
    margin-right: 10px;
    .ant-tree {
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
    // 鼠标滑动效果
    .ant-tree-treenode:hover {
        background: ${(props): string => props.theme.bgColorLight};

        .btn-box {
            display: flex;
        }
    }
    // 选中效果
    .ant-tree-treenode-selected {
        background: ${(props): string => props.theme.bgColorLight};
    }
    // 比对数据
    .baseline{
        color: ${(props): string => props.theme.primaryColor};
    }
    .comparison{
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

// 目录
const Contents = observer(({ session }: {session: Session}) => {
    const contents = useMemo<TreeDataNode[]>(() => session.dataSources.map((dataSource, dataSourceIndex) => ({
        key: dataSource.projectName,
        isLeaf: false,
        icon: <LocalImportIcon/>,
        title: <span className={'content-body'}>
            <span className={'content-text'}>{dataSource.projectName}</span>
            <div className={'btn-box'} onClick={(e): void => e.stopPropagation()}>
                <AddIcon onClick={(): void => { session.actionListener = { type: SessionAction.ADD_DATA_UNDER_PROJECT, value: dataSource.projectName }; }}/>
                <DeleteConfirm isProject={true} projectIndex={dataSourceIndex} />
            </div>
        </span>,
        children: dataSource.dataPath?.map((path, dataPathIndex) => ({
            key: `${dataSource.projectName}-${path}`,
            isLeaf: true,
            title: <span className={`content-body ${getNodeClass(session, { projectName: dataSource.projectName, filePath: path })}`}>
                <span className={'content-text can-right-click'}
                    onContextMenu={(): void => handleRightClick({ projectName: dataSource.projectName, filePath: path })}>{path}</span>
                <div className={'btn-box'} onClick={(e): void => e.stopPropagation()}>
                    <DeleteConfirm isProject={false} projectIndex={dataSourceIndex} dataPathIndex={dataPathIndex}/>
                </div>
            </span>,
        })),
    })), [session.dataSources, JSON.stringify(session.compareSet)]);

    // 展开情况: 默认展开所有工程，新导入工程默认展开
    const allProjectKeys = contents.map(item => item.key);
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

    // 点击目录
    const handleSelect = (keys: React.Key[], { node }: {node: EventDataNode<TreeDataNode>}): void => {
        const { activeDataSource, dataSources } = session;
        const [,projectIndex, dataPathIndex] = node.pos.split('-').map(index => Number(index));
        const dataSource: DataSource = dataSources[projectIndex];
        // 如果点击其它工程或者其它工程下文件
        if (dataSource.projectName !== activeDataSource.projectName) {
            handleProjectAction({ action: ProjectAction.SWITCH_PROJECT, dataSource, isConflict: false });
        }
        // 如果点击的是文件
        if (node.isLeaf) {
            runInAction(() => {
                session.activeDataSource = {
                    remote: LOCAL_HOST,
                    port: PORT,
                    projectName: dataSource.projectName,
                    dataPath: [dataSource.dataPath[dataPathIndex]],
                };
            });
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
    return <ContentsContainer>
        <Tree
            blockNode={true}
            showIcon={true}
            treeData={contents}
            selectedKeys={selectedKeys}
            multiple={true}
            expandedKeys={expandedKeys}
            onSelect={handleSelect}
            onExpand={handleExpand}
        />
    </ContentsContainer>
    ;
});

export default Contents;
