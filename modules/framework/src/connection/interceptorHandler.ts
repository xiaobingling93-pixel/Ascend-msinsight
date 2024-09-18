/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { type ResponseInterceptor } from './defs';
import { type DataSource } from '../centralServer/websocket/defs';
import { customConsole as console } from 'ascend-utils';

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
        console.log(event);
    } catch (error) {
        console.error(error);
    }
};
