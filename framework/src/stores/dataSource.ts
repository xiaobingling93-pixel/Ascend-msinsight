/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import {computed, ref, type Ref} from 'vue';
import {defineStore} from 'pinia';
import {
    type DataSource,
    LOCAL_HOST,
    PORT,
    ProjectActionEnum,
    type ProjectDirectory
} from '@/centralServer/websocket/defs';
import type {TreeNodeType} from '@/components/MenuTree/types';
import {addDataPath, connectRemote, disconnectRemote, isExistedRemote, request} from '@/centralServer/server';
import connector from '@/connection';
import {useSession} from './session';
import {ElMessage} from 'element-plus';
import {console} from '@/utils/console';
import {t} from '@/i18n';
import {ProjectErrorType} from '@/utils/enmus';
import {useLoading} from '@/hooks/useLoading';
import {type UpdateProjectExplorerParam} from '@/stores/resourceComp';
import {useCompareConfig} from '@/stores/compareConfig';
import {LocalStorageKeys, localStorageService} from '@/utils/local-storage';

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

function arraysEqual<T>(a: T[], b: T[]): boolean {
    return a.length === b.length && a.every((value) => b.includes(value));
}

export const useDataSources = defineStore('dataSources', () => {
    const { session } = useSession();
    const dataSources = ref<DataSource[]>([{ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] }]);
    // 当前选中数据，projectName为选中一级目录，dataPath为二级目录，为空时表示选中的是一级目录，不为空则表示选中的是二级目录
    const lastDataSource = ref<DataSource>({ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] });
    const compareConfig = useCompareConfig();
    const loadingMask = useLoading();
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

    const confirm = async (dataSource: DataSource, isConflict: boolean, action: ProjectActionEnum): Promise<void> => {
        // 如果目标内容就是当前选中内容，则不做任何处理直接返回
        if (dataSource.projectName === lastDataSource.value.projectName && arraysEqual(dataSource.dataPath, lastDataSource.value.dataPath)) {
            loadingMask.close();
            return;
        }
        // 如果是只是切换二级目录，则只修改当前选中内容
        if (action === ProjectActionEnum.TRANSFER_PROJECT && dataSource.projectName === lastDataSource.value.projectName) {
            lastDataSource.value = dataSource;
            compareConfig.cancelCompareData();
            loadingMask.close();
            return;
        }
        if (checkExistedServer(dataSource)) {
            loadingMask.close();
            return;
        }
        await compareConfig.cancelBaselineData();
        if (dataSource.dataPath.length > 0) {
            localStorageService.setItem(LocalStorageKeys.LAST_FILE_PATH, dataSource.dataPath[0]);
        }
        if (isExistedRemote(dataSource)) {
            addDataPath(dataSource, action, isConflict);
        } else {
            const isSuccess = await connectRemote(dataSource);
            if (isSuccess) {
                loadingMask.open({});
                connector.send({
                    event: 'remote/import',
                    body: { dataSource, isConflict, action },
                });
            }
        }
    };

    const menuTree = computed<TreeNodeType[]>(() => {
            return dataSources.value.filter(dataSource => dataSource.dataPath.length !== 0).map(dataSource => ({
                id: dataSource.projectName,
                projectName: '',
                label: dataSource.projectName,
                children: dataSource.dataPath.map(data => ({ id: `${dataSource.projectName}-${data}`, projectName: dataSource.projectName, label: data})),
                cancelable: true,
            }));
        }
    );

    const removeAllProjects = async (): Promise<void> => {
        session.loading = true;
        await request({ remote: LOCAL_HOST, port: PORT }, 'global', {
            command: 'files/clearProjectExplorer',
        }).finally(() => {
            session.loading = false;
        });
        connector.send({
            event: 'remote/reset',
            body: {},
        });
        session.reset(true);
        dataSources.value = [];
        lastDataSource.value = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] };
    };

    // 删除单个工程
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
                await request(dataSource, 'timeline', { command: 'remote/reset' });
                lastDataSource.value = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] };
            }
            if (compareConfig.baselineDataInfo.projectName === dataSource.projectName) {
                await compareConfig.cancelBaselineData();
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

    // 删除工程二级目录
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
            if (compareConfig.baselineDataInfo.projectName === dataSource.projectName &&
                compareConfig.baselineDataInfo.filePath === singleDataPath) {
                await compareConfig.cancelBaselineData();
            }
            await request(dataSource, 'global', {command: 'files/deleteProjectExplorer',
                params: {projectName: dataSource.projectName, dataPath: [singleDataPath]}});
        } catch {
            console.log('removeSingle error');
        }
        dataSources.value[parentIndex].dataPath.splice(index, 1);

        if (lastDataSource.value.projectName === dataSource.projectName && lastDataSource.value.dataPath[0] === singleDataPath) {
            lastDataSource.value = { ...dataSource, dataPath: [dataSource.dataPath[0]] };
        }
    };

    const updateProjectName = async (oldProjectName: string, newProjectName: string): Promise<boolean> => {
        try {
            const projectNameIdx = dataSources.value.findIndex((item) => item.projectName === newProjectName);
            if (projectNameIdx !== -1) {
                ElMessage.warning(t('Duplicate Project') as string);
                return false;
            }
            // 请求后端 更新数据
            await request({ remote: LOCAL_HOST, port: PORT }, 'global', {
                command: 'files/updateProjectExplorer',
                params: {
                    oldProjectName,
                    newProjectName,
                },
            });
            compareConfig.updateProjectName(oldProjectName, newProjectName);
            const idx = dataSources.value.findIndex((item) =>
                item.projectName === oldProjectName);
            if (idx !== -1) {
                dataSources.value[idx].projectName = newProjectName;
            }
            if (lastDataSource.value.projectName === oldProjectName) {
                lastDataSource.value.projectName = newProjectName;
            }
            connector.send({
                event: 'updateProjectName',
                body: { oldProjectName, newProjectName },
            });
            return true;
        } catch {
            ElMessage.warning(t('Update Project Name Failed') as string);
            return false;
        }
    };

    const updateProjectExplorer = (param: UpdateProjectExplorerParam): void => {
        try {
            // 更新dataSource
            const dataSource = { remote: LOCAL_HOST, port: PORT, projectName: param.projectName, dataPath: param.subdirectoryForUpdate };
            if (param.projectAction === ProjectActionEnum.ADD_FILE) {
                mergeDataSource(dataSources, dataSource, param.isConflict);
                // 如果存在冲突 或 切换的子目录存在多个，则选中一级目录
                if (param.isConflict || param.subdirectoryForUpdate.length > 1) {
                    lastDataSource.value = { remote: LOCAL_HOST, port: PORT, projectName: param.projectName, dataPath: [param.subdirectoryForUpdate[0]] };
                    return;
                }
                // 导入项目时，如果项目发生了切换，或原本选的为二级目录，则更新当前选中目录
                if (lastDataSource.value.projectName !== dataSource.projectName || lastDataSource.value.dataPath.length > 0) {
                    lastDataSource.value = dataSource;
                    return;
                }
            }
            // 切换项目时，以请求内容为准
            if (param.projectAction === ProjectActionEnum.TRANSFER_PROJECT) {
                lastDataSource.value = { remote: LOCAL_HOST, port: PORT, projectName: param.projectName, dataPath: param.subdirectory };
            }
        } catch {
            ElMessage.warning(t('Update Project Explorer Failed') as string);
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

    return {
        menuTree,
        remove,
        confirm,
        removeSingle,
        lastDataSource,
        updateProjectName,
        initProjectName,
        checkProjectValid,
        updateProjectExplorer,
        removeAllProjects,
    };
});
