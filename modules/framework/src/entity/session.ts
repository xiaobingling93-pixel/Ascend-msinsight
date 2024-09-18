/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
import { type DataSource, LOCAL_HOST, PORT } from '../centralServer/websocket/defs';

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';

    lastDataSource: DataSource = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] };

    constructor() {
        makeAutoObservable(this);
    }
}
