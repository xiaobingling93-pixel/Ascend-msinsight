/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { request } from '@/centralServer/server';
import { ImportResultBody, LayerType, Project, ProjectDirectory } from '@/centralServer/websocket/defs';
import { File } from '@/entity/session';
import { ProjectAction } from '@/utils/enum';
import { ErrorMsg } from '@/centralServer/websocket/connection';

export interface ImportProjectParams extends Record<string, unknown> {
    projectName: string;
    path: string[];
    selectedFileType?: LayerType;
    selectedFilePath?: string;
    projectAction: ProjectAction;
    isConflict: boolean;
}

export interface CardInfo {
    cardName: string;
    rankId: string;
    result: boolean;
    cardPath: string;
    host?: string;
    dataPathList: string[];
}

export interface ImportResult {
    reset: boolean;
    isPending: boolean;
    isSimulation: boolean;
    isIpynb: boolean;
    isCluster: boolean;
    result: CardInfo[];
}

/**
 * 获取文件路径下所有文件、文件夹
 * @returns ProjectDirectory[]
 */
export const getFiles = async (path = ''): Promise<unknown> => {
    return request('global', {
        command: 'files/get',
        params: { path },
    });
};

/**
 * 检查项目是否安全
 * @returns ProjectError
 */
export const checkProjectValid = async (params: {projectName: string;dataPath: string[]}): Promise<unknown> => {
    return request('global', {
        command: 'files/checkProjectValid',
        params,
    });
};

/**
 * 清理Timeline
 */
export const resetTimeline = async (): Promise<unknown> => {
    return request('timeline', { command: 'remote/reset' });
};

export const importProject = async (params: ImportProjectParams): Promise<ImportResultBody | ErrorMsg> => {
    return request<ImportResultBody>('timeline', {
        command: 'import/action',
        params,
    });
};

/**
 * 获取历史导入的文件
 * @returns ProjectDirectory[]
 */
export const getHistoryProject = async (): Promise<{projectDirectoryList: ProjectDirectory[]} | ErrorMsg> => {
    return request<{projectDirectoryList: ProjectDirectory[]}>('global', {
        command: 'files/getProjectExplorer',
        params: {},
    });
};

/**
 * 修改工程名
 */
export const updateProjectName = async (oldProjectName: string, newProjectName: string): Promise<unknown> => {
    return request('global', {
        command: 'files/updateProjectExplorer',
        params: { oldProjectName, newProjectName },
    });
};

/**
 *
 * 删除项目
 */
export const deleteProject = async (project: Project): Promise<unknown> => {
    return request('global', {
        command: 'files/deleteProjectExplorer',
        params: { projectName: project.projectName, dataPath: [] },
    });
};

export const clearProjects = async (projectNameList: React.Key[] = []): Promise<unknown> => {
    return request('global', {
        command: 'files/clearProjectExplorer',
        params: { projectNameList },
    });
};
/**
 * 移除文件
 */
export const deleteDataPath = async (project: { projectName: string; dataPath: string[] }): Promise<unknown> => {
    return request('global', {
        command: 'files/deleteProjectExplorer',
        params: { projectName: project.projectName, dataPath: project.dataPath },
    });
};

/**
 * 设置基线数据
 */
export const setBaseline = async (file: File): Promise<unknown> => {
    return request('global', {
        command: 'global/setBaseline',
        params: { ...file },
    });
};

/**
 * 取消基线数据
 */
export const cancelBaseline = async (): Promise<unknown> => {
    return request('global', {
        command: 'global/cancelBaseline',
        params: {},
    });
};

/**
 * 获取插件配置
 */
export const getModuleConfig = async (): Promise<unknown> => {
    return request('global', {
        command: 'moduleConfig/get',
        params: {},
    });
};
