/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { defineStore } from 'pinia';
import { ref } from 'vue';
import { LOCAL_HOST, PORT, ProjectActionEnum } from '@/centralServer/websocket/defs';
import { useDataSources } from '@/stores/dataSource';
import { ElMessage } from 'element-plus';
import { t } from '@/i18n';
import { request } from '@/centralServer/server';

const UNDERLINE: string = '_';
export interface DataInfo {
    projectName: string;
    filePath: string;
    rankId: string;
}

const getUniqueKeyByProjectInfo = (projectName: string, filePath: string): string => {
    return projectName + UNDERLINE + filePath;
};

export const useCompareConfig = defineStore('compareConfig', () => {
    // dataInfoMap维护当前导入数据的路径、rankid信息；baselineDataInfo维护基线数据信息；compareDataInfo维护对比数据信息
    const dataInfoMap = ref<Map<string, DataInfo>>(new Map<string, DataInfo>());
    const baselineDataInfo = ref<DataInfo>({ projectName: '', filePath: '', rankId: '' });
    const compareDataInfo = ref<DataInfo>({ projectName: '', filePath: '', rankId: '' });
    const isCompareStatus = ref<boolean>(false);

    const updateDataInfoMap = (
        projectAction: ProjectActionEnum,
        projectName: string,
        rankInfoList: Array<{ rankId: string; dataPathList: string[] }>,
    ): void => {
        // 涉及更新都取消对比数据的设置
        cancelCompareData();
        // 当前选中为一级目录，并在该目录下新增文件时，不进行重置，其他情况都需要对Map进行重置
        const needReset =
            projectAction !== ProjectActionEnum.ADD_FILE ||
            useDataSources().lastDataSource.projectName !== projectName ||
            useDataSources().lastDataSource.dataPath.length > 0;
        if (needReset) {
            dataInfoMap.value.clear();
        }
        rankInfoList.forEach((rankInfo) => {
            if (rankInfo.dataPathList.length !== 0) {
                const filePath = rankInfo.dataPathList[0];
                const info = { projectName, filePath, rankId: rankInfo.rankId };
                dataInfoMap.value.set(getUniqueKeyByProjectInfo(projectName, filePath), info);
            }
        });
    };

    /**
     * 设置基线数据
     * @param projectName 项目名
     * @param filePath 路径
     */
    const setBaselineData = async (projectName: string, filePath: string): Promise<void> => {
        baselineDataInfo.value = { projectName, filePath, rankId: 'baseline' };
        const result: any = await request({ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] }, 'global', {
            command: 'global/setBaseline',
            params: { projectName, filePath },
        });
        if (result.errorMessage as string) {
            ElMessage.warning(result.errorMessage as string);
        } else {
            baselineDataInfo.value.rankId = result.rankId;
        }
    };

    /**
     * 取消基线数据
     */
    const cancelBaselineData = async (): Promise<void> => {
        // 取消baseline同时也取消对比数据
        baselineDataInfo.value = { projectName: '', filePath: '', rankId: '' };
        compareDataInfo.value = { projectName: '', filePath: '', rankId: '' };
        isCompareStatus.value = false;
        await request({ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] }, 'global', {
            command: 'global/cancelBaseline',
            params: {},
        });
    };

    /**
     * 取消对比数据
     */
    const cancelCompareData = (): void => {
        compareDataInfo.value = { projectName: '', filePath: '', rankId: '' };
        isCompareStatus.value = false;
    };

    /**
     * 设置对比数据
     * @param projectName 项目名
     * @param filePath 路径
     */
    const setCompareData = (projectName: string, filePath: string): void => {
        if (!dataInfoMap.value) {
            return;
        }
        if (!baselineDataInfo.value.rankId) {
            ElMessage.warning(t('Set baseline first') as string);
            return;
        }
        const key = getUniqueKeyByProjectInfo(projectName, filePath);
        // 必须被包含在当前选中目录中
        if (!dataInfoMap.value.has(key)) {
            ElMessage.warning(t('Set Compare Data Out Of Range') as string);
            return;
        }
        if (dataInfoMap.value.get(key)?.rankId === baselineDataInfo.value.rankId) {
            ElMessage.warning(t('Baseline Conflict') as string);
            return;
        }
        compareDataInfo.value = dataInfoMap.value.get(key) as DataInfo;
        isCompareStatus.value = true;
    };

    const getRankIdByProjectInfo = (projectName: string, filePath: string): string => {
        const key = getUniqueKeyByProjectInfo(projectName, filePath);
        if (dataInfoMap.value.has(key)) {
            const info = dataInfoMap.value.get(key);
            return info ? info.rankId : '';
        }
        return '';
    };

    const updateProjectName = (oldProjectName: string, newProjectName: string): void => {
        for (let [key, value] of dataInfoMap.value) {
            if (value.projectName === oldProjectName) {
                value.projectName = newProjectName;
                dataInfoMap.value.delete(getUniqueKeyByProjectInfo(oldProjectName, value.filePath));
                dataInfoMap.value.set(getUniqueKeyByProjectInfo(newProjectName, value.filePath), value);
            }
        }
        if (baselineDataInfo.value.projectName === oldProjectName) {
            baselineDataInfo.value.projectName = newProjectName;
        }
        if (compareDataInfo.value.projectName === oldProjectName) {
            compareDataInfo.value.projectName = newProjectName;
        }
    };

    return {
        baselineDataInfo,
        compareDataInfo,
        isCompareStatus,
        updateDataInfoMap,
        setBaselineData,
        setCompareData,
        getRankIdByProjectInfo,
        cancelCompareData,
        cancelBaselineData,
        updateProjectName,
    };
});
