/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { request } from '@/centralServer/server';
import { DataSource, GLOBAL_HOST, LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import { File } from '@/entity/session';

/**
 * 获取历史导入的文件
 * @returns ProjectDirectory[]
 */
export const getHistoryProject = async (): Promise<unknown> => {
    return request({ remote: LOCAL_HOST, port: PORT }, 'global', {
        command: 'files/getProjectExplorer',
        params: {},
    });
};
/**
 * 获取文件路径下所有文件、文件夹
 * @returns ProjectDirectory[]
 */
export const getFiles = async (path = ''): Promise<unknown> => {
    return request({ remote: LOCAL_HOST, port: PORT }, 'global', {
        command: 'files/get',
        params: { path },
    });
};

/**
 * 检查文件路径是否安全
 * @returns ProjectError
 */
export const checkProjectValid = async (dataSource: DataSource, params: {projectName: string;dataPath: string[]}): Promise<unknown> => {
    return request(dataSource, 'global', {
        command: 'files/checkProjectValid',
        params,
    });
};

/**
 * 修改工程名
 */
export const updateProjectName = async (oldProjectName: string, newProjectName: string): Promise<unknown> => {
    return request(GLOBAL_HOST, 'global', {
        command: 'files/updateProjectExplorer',
        params: { oldProjectName, newProjectName },
    });
};

/**
 * 清理Timeline
 */
export const resetTimeline = async (): Promise<unknown> => {
    return request(GLOBAL_HOST, 'timeline', { command: 'remote/reset' });
};

/**
 *
 * 删除项目
 */
export const deleteProject = async (dataSource: DataSource): Promise<unknown> => {
    return request(dataSource, 'global', {
        command: 'files/deleteProjectExplorer',
        params: { projectName: dataSource.projectName, dataPath: [] },
    });
};

export const clearProjects = async (projectNameList: React.Key[] = []): Promise<unknown> => {
    return request(GLOBAL_HOST, 'global', {
        command: 'files/clearProjectExplorer',
        params: { projectNameList },
    });
};
/**
 * 移除文件
 */
export const deleteDataPath = async (dataSource: DataSource): Promise<unknown> => {
    return request(dataSource, 'global', {
        command: 'files/deleteProjectExplorer',
        params: { projectName: dataSource.projectName, dataPath: dataSource.dataPath },
    });
};

/**
 * 设置基线数据
 */
export const setBaseline = async (file: File): Promise<unknown> => {
    return request(GLOBAL_HOST, 'global', {
        command: 'global/setBaseline',
        params: { ...file },
    });
};

/**
 * 取消基线数据
 */
export const cancelBaseline = async (): Promise<unknown> => {
    return request(GLOBAL_HOST, 'global', {
        command: 'global/cancelBaseline',
        params: {},
    });
};
