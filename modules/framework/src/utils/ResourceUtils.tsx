/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { FileIcon, FolderIcon } from 'ascend-icon';
import type { TreeDataNode } from 'antd';
import { checkProjectValid, getFiles } from '@/utils/RequestUtils';
import { DataSource } from '@/centralServer/websocket/defs';
import { ProjectError } from '@/utils/enum';
import localStorageService, { LocalStorageKey } from '@/utils/local-storage';
export interface ResourceItem {
    path: string;
    name: string;
    childrenFolders: ResourceItem[];
    childrenFiles: ResourceItem[];
    exist: boolean;
};

// 接口返回数据转为Tree组件数据结构
export const dealResource = (resource: ResourceItem): TreeDataNode[] => {
    const folders = (resource.childrenFolders ?? []).map(item => ({
        isLeaf: false,
        icon: <FolderIcon/>,
        title: <span id={item.path}>{item.name}</span>,
        key: item.path,
    }));
    const files = (resource.childrenFiles ?? [])?.map(item => ({
        isLeaf: true,
        icon: <FileIcon/>,
        title: <span id={item.path}>{item.name}</span>,
        key: item.path,
    }));
    return [...folders, ...files];
};

// 更新Tree组件子节点
export const updateTreeData = (list: TreeDataNode[], key: React.Key, children: TreeDataNode[]): TreeDataNode[] =>
    list.map(node => {
        if (node.key === key) {
            return {
                ...node,
                children,
            };
        }
        if (node.children) {
            return {
                ...node,
                children: updateTreeData(node.children, key, children),
            };
        }
        return node;
    });

// 文件/文件夹是否存在
export const fileExist = async (path: string): Promise<boolean> => {
    if (path === undefined) {
        return false;
    }
    const result = await getFiles(path) as ResourceItem;
    return result.exist;
};

// 文件/文件夹安全校验
export const checkPathValid = async({ path, dataSource }: { path: string; dataSource: DataSource}):
Promise<ProjectError> => {
    // 检查文件是否存在
    const existed = await fileExist(path);
    if (!existed) {
        return ProjectError.FILE_NOT_EXIST;
    }
    // 是否在左侧已导入的数据中
    if (checkExistedDataSource(dataSource)) {
        return ProjectError.IMPORTED;
    }
    // 校验文件安全，如文件冲突、超大文件、软链接等
    const res = await checkProjectValid(dataSource, { projectName: dataSource.projectName ?? '', dataPath: dataSource.dataPath ?? [] });
    return (res as {result: ProjectError}).result;
};

// 文件是否已导入
export const checkExistedDataSource = (dataSource: DataSource): boolean => {
    const allProject: DataSource[] = [];
    const projectIndex = allProject.findIndex((item) =>
        item.remote === dataSource.remote &&
            item.port === dataSource.port &&
            item.projectName === dataSource.projectName);
    if (projectIndex === -1) {
        return false;
    }
    const pathIndex = (dataSource.dataPath ?? []).findIndex(path => allProject[projectIndex].dataPath?.includes(path));
    return pathIndex > -1;
};

// 上次导入文件路径
export const getLastFilePath = (): string => {
    return localStorageService.getItem(LocalStorageKey.LAST_FILE_PATH) ?? '';
};

// 修整输入的文件路径
// 1、去掉首尾空字符串
// 2、尾部多余的斜杠\ /，如D:\data\ -> D:\data
export const getTrimedPath = (inputPath: string): string => {
    return inputPath.trim().replace(/[^\\/][\\/]+$/, (match: string) => match.substr(0, 1));
};

// 查询目录树时，自动修整查询路径
// 1、修整
// 2、云环境默认路径
export const getSearchDir = (inputPath = ''): string => {
    const path = getTrimedPath(inputPath);
    if (path !== '') {
        return path;
    }
    // 云指定路径
    const isCloudEnv = window.location.pathname.includes('/proxy/');
    const defaultCloudDir = '/home/ma-user/work';
    return isCloudEnv ? defaultCloudDir : path;
};
