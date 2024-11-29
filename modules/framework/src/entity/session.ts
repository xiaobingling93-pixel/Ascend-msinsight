/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
import { type DataSource, LOCAL_HOST, PORT } from '../centralServer/websocket/defs';
import { SessionAction } from '@/utils/enum';

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    activeDataSource: DataSource = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] };
    isBinary: boolean | null = false;
    isCluster: boolean | null = false;
    isIpynb: boolean = false;
    actionListener: {type: SessionAction;value: string} = { type: SessionAction.NO_ACTION, value: '' };

    defaultConnected?: boolean;
    private _dataSources: DataSource[] = [];

    constructor() {
        makeAutoObservable(this);
    }

    get dataSources(): DataSource[] {
        return this._dataSources;
    }

    set dataSources(data: DataSource[]) {
        this._dataSources = data;
    }

    reset(remove?: boolean): void {
        this.isIpynb = false;
        this.isCluster = remove ? false : null;
        this.isBinary = remove ? false : null;
    }

    deleteDataSource(projectIndex: number): void {
        this._dataSources = this._dataSources.filter((dataSource, index) => index !== projectIndex);
    }

    deleteDataPath(projectIndex: number, dataPahtIndex: number): void {
        const dataSources = JSON.parse(JSON.stringify(this._dataSources));
        dataSources[projectIndex].dataPath.splice(dataPahtIndex, 1);
        this._dataSources = dataSources;
    }
}
