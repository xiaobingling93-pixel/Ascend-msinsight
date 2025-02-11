/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { store } from '@/store';
import { runInAction } from 'mobx';
import { ProjectAction } from './enum';
import { Rank } from '@/entity/session';

// 导入数据，更新卡信息
export const updateRankMap = (projectAction: ProjectAction, projectName: string, rankInfoList: Array<{ rankId: string; dataPathList: string[] }>): void => {
    const session = store.sessionStore.activeSession;
    const { activeDataSource } = session;

    // 当前选中为一级目录，并在该目录下新增文件时，不进行重置，其他情况都需要对Map进行重置
    const needReset = projectAction !== ProjectAction.ADD_FILE || activeDataSource.projectName !== projectName;
    const newRankMap = needReset ? new Map() : session.rankMap;
    rankInfoList.forEach((rankInfo) => {
        if (rankInfo.dataPathList.length !== 0) {
            const filePath = rankInfo.dataPathList[0];
            const info = { projectName, filePath, rankId: rankInfo.rankId };
            newRankMap.set(getUniqueKeyByProjectInfo(projectName, filePath), info);
        }
    });
    runInAction(() => {
        session.rankMap = newRankMap;
    });
};

// 修改项目名，更新卡信息
export const updateRankMapByProjectName = (oldProjectName: string, newProjectName: string): void => {
    const session = store.sessionStore.activeSession;
    const { rankMap } = session;
    for (const [, value] of rankMap) {
        if (value.projectName === oldProjectName) {
            value.projectName = newProjectName;
            rankMap.delete(getUniqueKeyByProjectInfo(oldProjectName, value.filePath));
            rankMap.set(getUniqueKeyByProjectInfo(newProjectName, value.filePath), value);
        }
    }
    runInAction(() => {
        session.rankMap = rankMap;
    });
};

const UNDERLINE: string = '_';
const getUniqueKeyByProjectInfo = (projectName: string, filePath: string): string => {
    return projectName + UNDERLINE + filePath;
};

// 查找对应文件卡信息
export const getRankInfo = ({ projectName, filePath }: {projectName: string; filePath: string}): Rank | undefined => {
    const session = store.sessionStore.activeSession;
    const { rankMap } = session;
    const key = getUniqueKeyByProjectInfo(projectName, filePath);
    return rankMap.get(key);
};

// 查找对应文件卡ID
export const getRankId = (file: {projectName: string; filePath: string}): string => {
    return getRankInfo(file)?.rankId ?? '';
};
