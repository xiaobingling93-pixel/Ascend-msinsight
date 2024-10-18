/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { defineStore } from 'pinia';
import { ref } from 'vue';
import { LOCAL_HOST, PORT, ProjectActionEnum } from '@/centralServer/websocket/defs';
import { useDataSources } from '@/stores/dataSource';
import { ElMessage } from 'element-plus';
import { t } from '@/i18n';
import {removeBaseline, request, sendDaseLineInfo} from '@/centralServer/server';

const UNDERLINE: string = '_';
export interface DataInfo {
    projectName: string;
    filePath: string;
    rankId: string;
    host?: string;
    cardName?: string;
}

export interface TimelineCardInfo {
    cardName: string;
    cardPath: string;
    host: string;
    rankId: string;
    result: boolean;
}

const getUniqueKeyByProjectInfo = (projectName: string, filePath: string): string => {
    return projectName + UNDERLINE + filePath;
};

export const useCompareConfig = defineStore('compareConfig', () => {
    // dataInfoMap维护当前导入数据的路径、rankid信息；baselineDataInfo维护基线数据信息；compareDataInfo维护对比数据信息
    const dataInfoMap = ref<Map<string, DataInfo>>(new Map<string, DataInfo>());
    const baselineDataInfo = ref<DataInfo>({ projectName: '', filePath: '', rankId: '' });
    const compareDataInfo = ref<DataInfo>({ projectName: '', filePath: '', rankId: '' });
    const dataSources = useDataSources();
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
            useDataSources().lastDataSource.projectName !== projectName;
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
        const oldDatasource = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] as string[] };
        removeBaseline(oldDatasource, baselineDataInfo.value.filePath);
        // 取消对比数据的设置
        cancelCompareData();
        const datasource = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] as string[] };
        const result: any = await request(datasource, 'global', {
            command: 'global/setBaseline',
            params: { projectName, filePath },
        });
        if (result.errorMessage as string) {
            ElMessage.warning(result.errorMessage as string);
        } else {
            const dataInfo = result as DataInfo;
            baselineDataInfo.value = { projectName, filePath, rankId: result.rankId };
            const timelineCardInfos = [] as TimelineCardInfo[];
            const timelineCardInfo = {} as TimelineCardInfo;
            timelineCardInfo.result = true;
            timelineCardInfo.cardName = dataInfo.cardName ?? '';
            timelineCardInfo.host = dataInfo.host ?? '';
            timelineCardInfo.rankId = dataInfo.rankId;
            timelineCardInfo.cardPath = baselineDataInfo.value.filePath;
            datasource.dataPath.push(baselineDataInfo.value.filePath);
            datasource.projectName = baselineDataInfo.value.projectName;
            timelineCardInfos.push(timelineCardInfo);
            sendDaseLineInfo(datasource, timelineCardInfos);
        }
    };

    /**
     * 取消基线数据
     */
    const cancelBaselineData = async (): Promise<void> => {
        const datasource = {
            remote: LOCAL_HOST,
            port: PORT,
            projectName: baselineDataInfo.value.projectName,
            dataPath: [] as string[],
        };
        datasource.dataPath.push(baselineDataInfo.value.filePath);
        removeBaseline(datasource, baselineDataInfo.value.filePath);
        // 取消baseline同时也取消对比数据
        baselineDataInfo.value = { projectName: '', filePath: '', rankId: '' };
        compareDataInfo.value = { projectName: '', filePath: '', rankId: '' };
        isCompareStatus.value = false;
        await request({ remote: LOCAL_HOST, port: PORT }, 'global', {
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
        if (projectName === baselineDataInfo.value.projectName && filePath === baselineDataInfo.value.filePath) {
            ElMessage.warning(t('Baseline Conflict') as string);
            return;
        }
        if (projectName !== dataSources.lastDataSource.projectName) {
            ElMessage.warning(t('Set Comparison Data Out Of Range') as string);
            return;
        }
        const key = getUniqueKeyByProjectInfo(projectName, filePath);
        const rankId = dataInfoMap.value.get(key)?.rankId || '';
        compareDataInfo.value = {projectName, filePath, rankId};
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
