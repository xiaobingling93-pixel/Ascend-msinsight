/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import styled from '@emotion/styled';
import { Tooltip } from 'antd';
import { Tree } from 'ascend-components';
import { HandleSingleDoubleClick } from 'ascend-utils';
import type { EventDataNode } from 'antd/lib/tree';
import type { DataNode } from 'rc-tree/lib/interface';
import { AddIcon, LocalImportIcon } from 'ascend-icon';
import { store } from '@/store';
import type { File, Session } from '@/entity/session';
import { type DataSource, FileOrDirectory, GLOBAL_HOST, LayerType, Project } from '@/centralServer/websocket/defs';
import { ProjectAction, SessionAction } from '@/utils/enum';
import { loadHistoryProject, handleProjectAction } from '@/utils/Project';
import DeleteConfirm from './DeleteConfirm';
import { isSameFile } from './ContextMenu';
import { useTranslation } from 'react-i18next';
import EditableText from './EditableText';
import CheckMenu from './CheckMenu';
import { getRankId } from '@/utils/Rank';
import { cancelCompareData, isInClusterCompare } from '@/utils/Compare';

interface ProjectTreeDataNode extends DataNode {
    layerType: LayerType;
    layerData: FileOrDirectory | Project;
}

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
        display: flex;
        flex: 1;
        min-width: 0;
        padding: 0;
        &:hover {
            background: none;
        }
        &.ant-tree-node-selected {
            background: none;
        }
    }
    // 勾选状态
    .ant-tree-title {
        display: inline-block;
        justify-content: space-between;
        flex: 1;
        min-width: 0;
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
        width: 30px;
        display: none;
    }
    .btn-box.leaf {
        width: 16px;
    }
    // 选中效果/鼠标滑动效果
    .ant-tree-treenode-selected, .ant-tree-treenode:hover {
        background: ${(props): string => props.theme.bgColorLight};

        .btn-box {
            display: flex;
        }
        .content-name {
            width: calc(100% - 30px);
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

const createCompareRankIdFuncWithProjectName = (projectName: string): (a: FileOrDirectory, b: FileOrDirectory) => number =>
    (a, b) => getRankId({ projectName, filePath: a.path }).localeCompare(getRankId({ projectName, filePath: b.path }));

const getTreeNode = (data: FileOrDirectory, projectName: string, projectIndex: number, session: Session, depth: number): ProjectTreeDataNode => {
    const isLeaf = depth >= 5 || data.children.length <= 0;
    const node: ProjectTreeDataNode = {
        key: `${projectName}-${data.path}`,
        layerType: data.type as LayerType,
        layerData: data,
        checkable: false,
        isLeaf,
        title: <Tooltip mouseEnterDelay={0.3} placement="bottom" title={data.path}>
            <span className={`content-body ${getNodeClass(session, { projectName, filePath: data.path })}`}>
                <span className={`content-text ${data.type === 'CLUSTER' ? '' : 'can-right-click'}`} onContextMenu={(): void => {
                    handleRightClick({ projectName, filePath: data.path });
                }}>
                    {data.type === 'CLUSTER' ? data.name : getFilePathName({ projectName, filePath: data.path })}
                </span>
                {data.type === 'CLUSTER' || (<div className={`btn-box ${isLeaf ? 'leaf' : ''}`} onClick={(e): void => e.stopPropagation()}>
                    <DeleteConfirm isProject={false} projectIndex={projectIndex} dataPath={data.path}/>
                </div>)}
            </span>
        </Tooltip>,
    };
    if (!isLeaf) {
        node.children = data.children.filter((child) => child.type !== undefined)
            .sort(createCompareRankIdFuncWithProjectName(projectName))
            .map((child) => getTreeNode(child, projectName, projectIndex, session, depth + 1));
    }
    return node;
};

// 目录树数据
const getTreeData = (session: Session): ProjectTreeDataNode[] => {
    return session.dataSources.map((dataSource, dataSourceIndex) => ({
        key: dataSource.projectName,
        layerType: 'PROJECT',
        layerData: dataSource,
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
        children: dataSource.children?.filter((child) => child.type !== undefined)
            .sort(createCompareRankIdFuncWithProjectName(dataSource.projectName))
            .map((child) => getTreeNode(child, dataSource.projectName, dataSourceIndex, session, 0)),
    }));
};

const getAllTreeNodeKeysWithoutLeaf = (treeNode: ProjectTreeDataNode[]): string[] => {
    const keys: string[] = [];
    const getKeys = (nodes: ProjectTreeDataNode[], depth: number): void => {
        if (depth >= 5) { return; }
        nodes.forEach((node) => {
            if (!node.isLeaf) {
                keys.push(node.key as string);
                getKeys(node.children as ProjectTreeDataNode[], depth + 1);
            }
        });
    };
    getKeys(treeNode, 0);
    return keys;
};

// 文件名
const getFilePathName = (file: File): string => {
    const rankId = getRankId(file);
    return `${rankId}${rankId === '' ? '' : ' : '}${file.filePath}`;
};

// 目录
const Contents = observer(({ session }: {session: Session}) => {
    const treeData = useMemo<ProjectTreeDataNode[]>(() => getTreeData(session), [session.dataSources, JSON.stringify(session.compareSet), session.rankMap]);
    // 展开情况: 默认展开所有工程，新导入工程默认展开
    const allProjectKeys = treeData.map(item => item.key);
    const [collapsedKeys, setCollapsedKeys] = useState(new Set());
    const expandedKeys = useMemo(() => {
        return getAllTreeNodeKeysWithoutLeaf(treeData).filter(item => !collapsedKeys.has(item));
    }, [collapsedKeys, treeData]);

    // 当前选中（打开）工程、文件
    const selectedKeys = useMemo(() => {
        if (session.activeDataSource.projectName !== '') {
            const { projectName, activeDataPath } = session.activeDataSource;
            return [projectName, `${projectName}-${activeDataPath}`];
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

    // 点击项目
    const handleNodeClick = (keys: React.Key[],
        { selected, selectedNodes, node }: { selected: boolean; selectedNodes: ProjectTreeDataNode[]; node: EventDataNode<ProjectTreeDataNode> }): void => {
        const { activeDataSource, dataSources } = session;
        if (node.pos.split('-').length < 2 || selectedNodes.length < 1) { return; }
        const [,projectIndex] = node.pos.split('-').map(index => Number(index));
        const dataSource: DataSource = dataSources[projectIndex];
        // 如果点击其它工程或者其它工程下文件
        if (dataSource.projectName !== activeDataSource.projectName) {
            handleProjectAction({
                action: ProjectAction.SWITCH_PROJECT,
                project: dataSource,
                isConflict: false,
                selectedRankPath: (selectedNodes[selectedNodes.length - 1].layerData as FileOrDirectory).path,
            });
        }
        // 如果点击的是文件
        if (node.isLeaf) {
            if (!selected) { return; } // 如果是取消选中文件
            runInAction(() => {
                session.activeDataSource = {
                    ...GLOBAL_HOST,
                    ...dataSource,
                    activeDataPath: (selectedNodes[selectedNodes.length - 1].layerData as FileOrDirectory).path,
                };
            });
            if (isInClusterCompare()) {
                return;
            }
            cancelCompareData();
        }
    };

    const handleSingleClick = (keys: React.Key[],
        nodeEvent: { selected: boolean; selectedNodes: ProjectTreeDataNode[]; node: EventDataNode<ProjectTreeDataNode>; nativeEvent: MouseEvent}): void => {
        // 区分正常点击项目或文件还是点击Tooltip中的内容
        const target = nodeEvent.nativeEvent.target as HTMLElement;
        if (target.className === 'ant-tooltip-inner') {
            return;
        }
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
    const handleExpand = (keys: React.Key[], { node, expanded }: {expanded: boolean;node: ProjectTreeDataNode}): void => {
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
        <Tree<ProjectTreeDataNode>
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
