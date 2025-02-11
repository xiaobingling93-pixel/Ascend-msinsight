/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { customConsole as console } from 'ascend-utils';
import { type ResponseInterceptor } from './defs';
import { type DataSource } from '../centralServer/websocket/defs';
import { ProjectAction } from '@/utils/enum';
import { updateProject } from '@/utils/Project';
import { updateRankMap } from '@/utils/Rank';

interface ImportActionBody {
    subdirectoryList: string[];
    result: Array<{ rankId: string; dataPathList: string[] }>;
}

export interface ImportActionResponse {
    dataSource: DataSource;
    body: ImportActionBody;
}

export const importActionHandler: ResponseInterceptor<ImportActionResponse> = (event, data): void => {
    try {
        if (typeof event.data.args.params.projectAction !== 'number' || typeof event.data.args.params.isConflict !== 'boolean') {
            return;
        }
        const projectAction = event.data.args.params.projectAction as ProjectAction;
        const hasConflict = event.data.args.params.isConflict as boolean;
        const projectName = data.dataSource.projectName;
        const dataPath = data.body.subdirectoryList;
        // 更新rank信息
        const ranInfoList = data.body.result;
        updateRankMap(projectAction, projectName, ranInfoList);
        // 更新项目目录
        updateProject({ projectAction, projectName, dataPath, hasConflict });
    } catch (error) {
        console.error(error);
    }
};
