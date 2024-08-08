/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { ref, computed, type Ref } from 'vue';
import { defineStore } from 'pinia';
import { type DataSource, LOCAL_HOST, PORT, type ProjectDirectory } from '@/centralServer/websocket/defs';
import type { TreeNodeType } from '@/components/MenuTree/types';
import {addDataPath, connectRemote, disconnectRemote, isExistedRemote, request} from '@/centralServer/server';
import connector from '@/connection';
import { useSession } from './session';
import {ElMessage} from 'element-plus';
import { console } from '@/utils/console';
import { t } from '@/i18n';
import { ProjectErrorType } from '@/utils/enmus';
import { useLoading } from '@/hooks/useLoading';

const mergeDataSource = (dataSources: Ref<DataSource[]>, dataSource: DataSource, isConflict: boolean): boolean => {
    const idx = dataSources.value.findIndex((item) =>
        item.remote === dataSource.remote && item.port === dataSource.port &&
        item.projectName === dataSource.projectName);
    if (idx === -1) {
        dataSources.value.push(dataSource);
        return false;
    }

    if (isConflict) {
        dataSources.value[idx].dataPath = dataSource.dataPath;
        return true;
    }
    const dataPathIdx = dataSource.dataPath.findIndex(path =>
        dataSources.value[idx].dataPath.includes(path));
    if (dataPathIdx === -1) {
        dataSources.value[idx].dataPath.push(...dataSource.dataPath);
    }
    return true;
};

const checkExistedDataSource = (dataSources: Ref<DataSource[]>, dataSource: DataSource): boolean => {
    const idx = dataSources.value.findIndex((item) =>
        item.remote === dataSource.remote && item.port === dataSource.port &&
        item.projectName === dataSource.projectName);
    if (idx === -1) {
        return false;
    }
    const dataPathIdx = dataSource.dataPath.findIndex(path =>
        dataSources.value[idx].dataPath.includes(path));
    return dataPathIdx !== -1;
};

export const useDataSources = defineStore('dataSources', () => {
    const { session } = useSession();
    const dataSources = ref<DataSource[]>([{ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] }]);
    const lastDataSource = ref<DataSource>({ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] });
    function checkExistedServer(dataSource: DataSource, importMethod?: 'drag', result?: any): boolean {
        if (session.isReset) {
            session.reset();
        }
        if (lastDataSource.value?.projectName !== dataSource.projectName) {
            return false;
        }
        return dataSource.dataPath.length > 0 && lastDataSource.value?.dataPath.includes(dataSource.dataPath[0]);
    };

    const checkProjectValid = async (dataSource: DataSource): Promise<ProjectErrorType> => {
        if (checkExistedDataSource(dataSources, dataSource)) {
            // 检查文件是否已经存在，如果存在直接结束,视为无冲突
            return ProjectErrorType.NO_ERRORS;
        }
        try {
            const res = await request(dataSource, 'global', {
                command: 'files/checkProjectValid',
                params: {
                    projectName: dataSource.projectName,
                    dataPath: dataSource.dataPath,
                },
            });
            return (res as {result: ProjectErrorType.NO_ERRORS}).result;
        } catch {
            console.log('checkProjectValid error');
            return ProjectErrorType.NO_ERRORS;
        }
    };

    const confirm = async (dataSource: DataSource, isConflict: boolean): Promise<void> => {
        if (dataSource.projectName === lastDataSource.value?.projectName && dataSource.dataPath.length === 0) {
            return;
        }
        if (checkExistedServer(dataSource)) {
            return; 
        }
        mergeDataSource(dataSources, dataSource, isConflict);
        if (isExistedRemote(dataSource)) {
            addDataPath(dataSource);
        } else {
            const isSuccess = await connectRemote(dataSource);
            if (isSuccess) {
                useLoading().open({});
                connector.send({
                    event: 'remote/import',
                    body: { dataSource },
                });
            }
        }
        lastDataSource.value = dataSource;
    };

    const menuTree = computed<TreeNodeType[]>(() => {
            // 树状目录索引值，从1开始计
            let index = 1;
            return dataSources.value.filter(dataSource => dataSource.dataPath.length !== 0).map(dataSource => ({
                id: index++,
                projectName: '',
                label: dataSource.projectName,
                children: dataSource.dataPath.map(data => ({ id:index++, projectName: dataSource.projectName, label: data})),
                cancelable: true,
            }));
        }
    );

    const remove = async (index: number): Promise<void> => {
        session.loading = true;
        const dataSource = dataSources.value[index];
        if (!dataSource) {
            return;
        }
        try {
            // 如果当前文件在删除项目中，则需要对当前页面进行清空处理
            if (lastDataSource.value?.projectName === dataSource.projectName) {
                connector.send({
                    event: 'remote/remove',
                    body: { dataSource },
                });
                session.reset(true);
                await request(dataSource, 'timeline', { command: 'remote/reset', params: {} });
                lastDataSource.value = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] };
            }
            // 发送请求给后端，清空该项目目录数据
            await request(dataSource, 'global', {command: 'files/deleteProjectExplorer',
                params: {projectName: dataSource.projectName, dataPath: []}});
            // dataSource内容更新
            dataSources.value.splice(index, 1);
            if (dataSource.remote !== LOCAL_HOST) {
                disconnectRemote(dataSource);
            }
            session.loading = false;
        } catch {
            console.log('remove error');
        }
    };

    const removeSingle = async (parentIndex: number, index: number): Promise<void> => {
        session.loading = true;
        const dataSource = dataSources.value[parentIndex];
        if (!dataSource) {
            return;
        }
        if (dataSource.dataPath.length === 1) {
            remove(parentIndex);
            return;
        }
        const singleDataPath = dataSource.dataPath[index];
        connector.send({
            event: 'remote/removeSingle',
            body: { dataSource, singleDataPath },
        });
        try {
            await request(dataSource, 'global', {command: 'files/deleteProjectExplorer',
                params: {projectName: dataSource.projectName, dataPath: [singleDataPath]}});
        } catch {
            console.log('removeSingle error');
        }
        dataSources.value[parentIndex].dataPath.splice(index, 1);
    };

    const updateProjectName = async (oldProjectName: string, newProjectName: string): Promise<boolean> => {
        try {
            const projectNameIdx = dataSources.value.findIndex((item) => item.projectName === newProjectName);
            if (projectNameIdx !== -1) {
                ElMessage.warning(t('Duplicate Project') as string);
                return false;
            }
            // 请求后端 更新数据
            await request({ remote: LOCAL_HOST, port: PORT, projectName: oldProjectName, dataPath: [] }, 'global', {
                command: 'files/updateProjectExplorer',
                params: {
                    oldProjectName,
                    newProjectName,
                },
            });
            const idx = dataSources.value.findIndex((item) =>
                item.projectName === oldProjectName);
            if (idx !== -1) {
                dataSources.value[idx].projectName = newProjectName;
            }
            if (lastDataSource.value.projectName === oldProjectName) {
                lastDataSource.value.projectName = newProjectName;
            }
            return true;
        } catch {
            ElMessage.warning(t('Update Project Name Failed') as string);
            return false;
        }
    };

    const initProjectName = async (projectDirectoryList: ProjectDirectory[]): Promise<void> => {
        dataSources.value = [];
        projectDirectoryList.forEach(item => {
            let source: DataSource = {
                remote: LOCAL_HOST, port: PORT, projectName: item.projectName, dataPath: item.fileName,
            };
            dataSources.value.push(source);
        });
    };

    return { menuTree, remove, confirm, removeSingle, lastDataSource, updateProjectName, initProjectName, checkProjectValid };
});
