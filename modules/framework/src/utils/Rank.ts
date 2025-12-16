/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import { store } from '@/store';
import { runInAction } from 'mobx';
import { ProjectAction } from './enum';
import { Rank } from '@/entity/session';
import { ImportRankInfo } from '@/centralServer/websocket/defs';

// 导入数据，更新卡信息
export const updateRankMap = (projectAction: ProjectAction, projectName: string, rankInfoList: ImportRankInfo[]): void => {
    const session = store.sessionStore.activeSession;
    const { activeDataSource } = session;

    // 当前选中为一级目录，并在该目录下新增文件时，不进行重置，其他情况都需要对Map进行重置
    const needReset = projectAction !== ProjectAction.ADD_FILE || activeDataSource.projectName !== projectName;
    const newRankMap = needReset ? new Map() : session.rankMap;
    rankInfoList.forEach((rankInfo) => {
        const filePath = rankInfo.filePath;
        const info = { projectName, filePath, rankId: rankInfo.rankId };
        newRankMap.set(getUniqueKeyByProjectInfo(projectName, filePath, rankInfo.rankId), info);
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
            rankMap.delete(getUniqueKeyByProjectInfo(oldProjectName, value.filePath, value.rankId));
            rankMap.set(getUniqueKeyByProjectInfo(newProjectName, value.filePath, value.rankId), value);
        }
    }
    runInAction(() => {
        session.rankMap = rankMap;
    });
};

const UNDERLINE: string = '_';
const getUniqueKeyByProjectInfo = (projectName: string, filePath: string, rankId?: string): string => {
    return projectName + UNDERLINE + filePath + UNDERLINE + (rankId ?? '');
};

// 查找对应文件卡信息
export const getRankInfo = ({ projectName, filePath, rankId }: {projectName: string; filePath: string; rankId?: string}): Rank | undefined => {
    const session = store.sessionStore.activeSession;
    const { rankMap } = session;
    const key = getUniqueKeyByProjectInfo(projectName, filePath, rankId);
    return rankMap.get(key);
};
