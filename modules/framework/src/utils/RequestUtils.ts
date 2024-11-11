/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { request } from '@/centralServer/server';
import { DataSource, LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';

/**
 * 获取历史导入的文件
 * @returns ProjectDirectory[]
 */
export const getHistoryProject = async (): Promise<unknown> => {
    return request({ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] }, 'global', {
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
