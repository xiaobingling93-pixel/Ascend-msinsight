/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { customConsole as console } from 'ascend-utils';
import type { NotificationInterceptor, ResponseInterceptor } from './defs';
import { type DataSource } from '../centralServer/websocket/defs';
import { ProjectAction } from '@/utils/enum';
import { updateProject } from '@/utils/Project';
import { updateRankMap } from '@/utils/Rank';
import { updateDataScene } from '../components/TabPane/Index';
import { updateSession } from '@/connection/notificationHandler';
import { store } from '@/store';

interface ImportActionBody {
    subdirectoryList: string[];
    result: Array<{ rankId: string; dataPathList: string[] }>;
    isBinary: boolean;
    isCluster: boolean;
    isIpynb: boolean;
    isPending: boolean;
    isSimulation: boolean;
    isOnlyTraceJson: boolean;
    reset: boolean;
}

export interface ImportActionResponse {
    dataSource: DataSource;
    body: ImportActionBody;
}

interface MemoryResult {
    hasMemory: boolean;
    rankId: string;
}
interface ParseMemoryNotification {
    memoryResult: MemoryResult[];
}

export const importActionHandler: ResponseInterceptor<ImportActionResponse> = (event, data): void => {
    try {
        if (typeof event.data.args.params.projectAction !== 'number' || typeof event.data.args.params.isConflict !== 'boolean') {
            return;
        }
        const projectAction = event.data.args.params.projectAction as ProjectAction;
        const hasConflict = event.data.args.params.isConflict as boolean;
        const projectName = data.dataSource.projectName;
        const subdirectory = event.data.remote.dataPath as string[];
        const dataPath = data.body.subdirectoryList;
        // 更新场景
        updateDataScene(data.body);
        // 更新rank信息
        const ranInfoList = data.body.result;
        updateRankMap(projectAction, projectName, ranInfoList);
        // 更新项目目录
        updateProject({ projectAction, projectName, dataPath, hasConflict, subdirectory });
    } catch (error) {
        console.error(error);
    }
};

export const parseMemorySuccessHandler: NotificationInterceptor<ParseMemoryNotification> = (data): void => {
    const session = store.sessionStore.activeSession;
    const memoryRankIds: string[] = [...session.memoryRankIds];
    data.memoryResult.forEach((item) => {
        if (!memoryRankIds.includes(item.rankId) && item.hasMemory) {
            memoryRankIds.push(item.rankId);
        }
    });
    updateSession({ memoryRankIds });
};
