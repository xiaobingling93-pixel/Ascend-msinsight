/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { runInAction } from 'mobx';
import { customConsole as console } from 'ascend-utils';
import { LocalStorageKey, localStorageService } from 'ascend-local-storage';
import connector from '@/connection';
import { store } from '@/store';
import { ProjectAction } from '@/utils/enum';
import { DataSource, LOCAL_HOST, PORT, ProjectDirectory } from '@/centralServer/websocket/defs';
import { addDataPath, connectRemote, isExistedRemote } from '@/centralServer/server';
import {
    deleteDataPath, deleteProject, resetTimeline, getHistoryProject, updateProjectName as requestUpdateProjectName,
    clearProjects,
} from '@/utils/Request';
import i18n from 'ascend-i18n';
import { message as Message } from 'antd';
import { isProjectNameExisted, updateDataSourceName } from '@/utils/Resource';
import { sendReset, sendUpdateProjectName } from '@/connection/sendNotification';
import { cancelBaselineData, updateProjectNameHandler } from '@/utils/Compare';
import { updateRankMapByProjectName } from '@/utils/Rank';
import { openLoading, closeLoading } from '@/utils/useLoading';

export interface UpdateProjectParam {
    projectAction: ProjectAction;
    projectName: string;
    dataPath: string[];
    hasConflict: boolean;
    subdirectory: string[];
}

// 历史项目
export const loadHistoryProject = async(): Promise<void> => {
    const session = store.sessionStore.activeSession;
    const sources: DataSource[] = [];
    const result = await getHistoryProject() as {projectDirectoryList: ProjectDirectory[]};
    const projectDirectoryList = result?.projectDirectoryList ?? [];
    projectDirectoryList.forEach(item => {
        const source: DataSource = { remote: LOCAL_HOST, port: PORT, projectName: item.projectName, dataPath: item.fileName };
        sources.push(source);
    });
    runInAction(() => {
        session.dataSources = sources;
    });
};

// 导入数据、切换项目
export async function handleProjectAction({ action, dataSource: orginDataSource, isConflict, selectedPath }:
{action: ProjectAction;dataSource: DataSource;isConflict: boolean;selectedPath?: string}): Promise<void> {
    openLoading();
    const session = store.sessionStore.activeSession;
    runInAction(async() => {
        const { activeDataSource, dataSources } = session;
        const dataSource = { ...orginDataSource };

        if (session.isReset) {
            session.reset();
        }

        // 如果目标内容就是当前选中内容，则不做任何处理直接返回
        if (dataSource.projectName === activeDataSource.projectName && arraysValueEqual(dataSource.dataPath, activeDataSource.dataPath)) {
            return;
        }

        // 切换项目
        if (action === ProjectAction.SWITCH_PROJECT) {
            const firstDataPath = selectedPath ?? dataSources.find(data => data.projectName === dataSource.projectName)?.dataPath[0];
            if (firstDataPath !== undefined) {
                dataSource.dataPath = [firstDataPath];
            }
        }
        if (!isExistedRemote(dataSource)) {
            const connected = await connectRemote(dataSource);
            if (!connected) {
                return;
            }
        }
        addDataPath(dataSource, action, isConflict);

        // 保存文件路径
        const path = dataSource.dataPath[0].includes(dataSource.projectName) ? dataSource.projectName : dataSource.dataPath[0];
        localStorageService.setItem(LocalStorageKey.LAST_FILE_PATH, path);
    });
    cancelBaselineData();
    closeLoading();
}
// 允许2个数组值重复或乱序
function arraysValueEqual<T>(a: T[], b: T[]): boolean {
    return a.length === b.length && a.every((value) => b.includes(value));
}

// 项目更新
// 1、更新项目目录
// 2、设置为打开(选中）项目
export const updateProject = ({ projectAction, projectName, dataPath, hasConflict, subdirectory }: UpdateProjectParam): void => {
    const session = store.sessionStore.activeSession;
    runInAction(() => {
        try {
            if (projectAction === ProjectAction.ADD_FILE) {
                // 更新项目目录
                const dataSource = { remote: LOCAL_HOST, port: PORT, projectName, dataPath };
                session.dataSources = getMergedDataSources(session.dataSources, dataSource, hasConflict);
                // 如果存在冲突 或 切换的子目录存在多个，则选中一级目录
                if (hasConflict || dataPath.length > 1) {
                    session.activeDataSource = { ...dataSource, dataPath: [dataPath[0]] };
                }
                // 导入项目时，如果项目发生了切换，或原本选的为二级目录，则更新当前选中目录
                if (session.activeDataSource.projectName !== dataSource.projectName || session.activeDataSource.dataPath.length > 0) {
                    session.activeDataSource = dataSource;
                    return;
                }
            }
            if (projectAction === ProjectAction.SWITCH_PROJECT) {
                session.activeDataSource = { remote: LOCAL_HOST, port: PORT, projectName, dataPath: subdirectory };
            }
        } catch {
            console.error('Update Project Explorer Failed');
        }
    });
};

// 获取合并目录
function getMergedDataSources(oldDataSources: DataSource[], dataSource: DataSource, hasConflict: boolean): DataSource[] {
    const dataSources = JSON.parse(JSON.stringify(oldDataSources)) as DataSource[];
    const projectIndex = dataSources.findIndex((item) =>
        item.remote === dataSource.remote && item.port === dataSource.port && item.projectName === dataSource.projectName);
    // 项目不存在，新增项目
    if (projectIndex === -1) {
        dataSources.push(dataSource);
    } else {
        // 项目存在，在已有项目下增加文件
        // 有冲突，替换项目的子文件
        if (hasConflict) {
            dataSources[projectIndex].dataPath = dataSource.dataPath;
        } else {
            // 新导入文件全部不在当前项目下
            const oldDataPath = dataSources[projectIndex].dataPath;
            const dataPathIdx = dataSource.dataPath.findIndex(path => oldDataPath.includes(path));
            if (dataPathIdx === -1) {
                dataSources[projectIndex].dataPath.push(...dataSource.dataPath);
            }
        }
    }
    return dataSources;
}

// 移除项目
export const removeProject = (projectIndex: number): void => {
    const session = store.sessionStore.activeSession;
    runInAction(async() => {
        try {
            const dataSource = session.dataSources[projectIndex];
            // 如果移除的是当前打开项目
            if (session.activeDataSource.projectName === dataSource.projectName) {
                // 通知各页签
                connector.send({ event: 'remote/remove', body: { dataSource } });
                // 通知后台，清理Timeline
                await resetTimeline();
                // 重置frame页面
                session.reset(true);
                // 设置当前打开（选中）项目空
                session.activeDataSource = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] };
            }
            // 通知后台
            await deleteProject(dataSource);
            // 目录更新
            session.deleteDataSource(projectIndex);
        } catch {
            console.error('remove error');
        }
    });
};

// 移除多个项目
export const removeProjects = async (projectNameList: React.Key[] = []): Promise<void> => {
    const session = store.sessionStore.activeSession;
    runInAction(async() => {
        try {
            // 通知后台
            await clearProjects(projectNameList);
            // 目录更新
            session.dataSources = session.dataSources.filter(dataSource => !projectNameList.includes(dataSource.projectName));
            // 如果删除项目包含当前打开项目或者全部删除
            if (projectNameList.length === 0 || projectNameList.includes(session.activeDataSource.projectName)) {
                // 通知页签
                sendReset();
                // 通知后台，清理Timeline
                await resetTimeline();
                // framework重置
                session.reset(true);
                // 当前选中置空
                session.activeDataSource = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] };
            }
        } catch {
            console.error('remove error');
        }
    });
};

// 移除文件
export const removeDataPath = (projectIndex: number, dataPathIndex: number): void => {
    const session = store.sessionStore.activeSession;
    runInAction(async() => {
        try {
            const dataSource = session.dataSources[projectIndex];
            // 项目只有一个文件
            if (dataSource.dataPath.length === 1) {
                removeProject(projectIndex);
                return;
            }
            const singleDataPath = dataSource.dataPath[dataPathIndex];
            // 通知各页签
            if (session.activeDataSource.projectName === dataSource.projectName) {
                connector.send({ event: 'remote/removeSingle', body: { dataSource, singleDataPath } });
            }
            // 通知后台
            await deleteDataPath({ ...dataSource, dataPath: [singleDataPath] });
            // 目录更新
            session.deleteDataPath(projectIndex, dataPathIndex);
            // 如果移除的是当前选中文件，默认选中第一个文件
            if (session.activeDataSource.projectName === dataSource.projectName && session.activeDataSource.dataPath[0] === singleDataPath) {
                session.activeDataSource = { ...dataSource, dataPath: [dataSource.dataPath[0]] };
            }
        } catch {
            console.error('removeSingle error');
        }
    });
};

// 修改工程名
export const updateProjectName = async (oldProjectName: string, newProjectName: string): Promise<boolean> => {
    try {
        const existed = isProjectNameExisted(newProjectName);
        if (existed) {
            Message.warning(i18n.t('Duplicate Project', { ns: 'framework' }));
            return false;
        }
        // 更新卡信息
        updateRankMapByProjectName(oldProjectName, newProjectName);
        // 通知后端
        await requestUpdateProjectName(oldProjectName, newProjectName);
        // 通知模块
        sendUpdateProjectName(oldProjectName, newProjectName);
        // 修改目录
        updateDataSourceName(oldProjectName, newProjectName);
        // 更新对比功能
        updateProjectNameHandler(oldProjectName, newProjectName);
        return true;
    } catch {
        Message.warning(i18n.t('Update Project Name Failed', { ns: 'framework' }));
        return false;
    }
};
