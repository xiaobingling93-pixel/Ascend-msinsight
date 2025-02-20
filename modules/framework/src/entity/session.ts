/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
import { type DataSource, LOCAL_HOST, PORT } from '../centralServer/websocket/defs';
import type { CompareData } from '@/utils/Compare';
import { SessionAction } from '@/utils/enum';

// Scene：数据场景,默认、集群、算子调优、Jupter
export type Scene = 'Default' | 'Cluster' | 'Compute' | 'Jupyter';

interface ContextMenu {
    visible: boolean;
}
export interface File {
    projectName: string;
    filePath: string;
    rankId?: string;
}

export interface Rank extends File {
    rankId: string;
    cardName: string;
    cardPath: string;
    host?: string;
}
export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    defaultConnected?: boolean;
    actionListener: {type: SessionAction;value: string} = { type: SessionAction.NO_ACTION, value: '' };
    // 加载状态
    loading: boolean = false;
    // 数据源/项目管理
    rankList: Rank[] = [];
    rankMap: Map<string, Rank> = new Map();
    // 场景
    isCluster: boolean | null = false;
    isBinary: boolean | null = false;
    isIpynb: boolean = false;
    ipynbUrl: string = '';
    isFullDb: boolean = false;
    // 解析状态
    parseCompleted: boolean = false;
    clusterCompleted: boolean = false;
    durationFileCompleted: boolean = false;
    // 模块数据
    startTime: number = -1;
    endTimeAll: number = -1;
    unitcount: number = 0;
    memoryRankIds: string[] = [];
    operatorRankIds: string[] = [];
    // 模块数据-算子调优
    coreList: string[] = [];
    sourceList: string[] = [];
    // 右键菜单
    contextMenu: ContextMenu = { visible: false };
    // 对比功能
    selectedFile: File = { projectName: '', filePath: '' };
    compareSet: {baseline: CompareData;comparison: CompareData} = {
        baseline: { projectName: '', filePath: '', rankId: '' },
        comparison: { projectName: '', filePath: '', rankId: '' },
    };

    // 需要下发给插件的url
    toIframeUrl: string = '';

    // 资源目录的编辑状态
    projectContentEditStatus: boolean = false;

    // 数据源/项目管理
    private _dataSources: DataSource[] = [];

    private _activeDataSource: DataSource = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] };

    constructor() {
        makeAutoObservable(this);
    }

    // 导入数据场景：默认、集群、算子调优、Jupter
    get scene(): Scene {
        let scene: Scene;
        if (this.isBinary) {
            scene = 'Compute';
        } else if (this.isCluster) {
            scene = 'Cluster';
        } else if (this.isIpynb) {
            scene = 'Jupyter';
        } else {
            scene = 'Default';
        }
        return scene;
    }

    // 对比状态
    get isCompareStatus(): boolean {
        const { baseline, comparison } = this.compareSet;
        return baseline.projectName !== '' && comparison.filePath !== '' && comparison.projectName !== '' && comparison.filePath !== '';
    }

    // 数据源/工程管理
    get dataSources(): DataSource[] {
        return this._dataSources;
    }

    get activeDataSource(): DataSource {
        return this._activeDataSource;
    }

    set dataSources(data: DataSource[]) {
        this._dataSources = data;
    }

    set activeDataSource(data: DataSource) {
        this._activeDataSource = data;
    }

    reset(remove?: boolean): void {
        this.isIpynb = false;
        this.isCluster = remove ? false : null;
        this.isBinary = remove ? false : null;
        this.toIframeUrl = '';
    }

    // 数据源管理
    deleteDataSource(projectIndex: number): void {
        this._dataSources = this._dataSources.filter((dataSource, index) => index !== projectIndex);
    }

    deleteDataPath(projectIndex: number, dataPahtIndex: number): void {
        const dataSources = JSON.parse(JSON.stringify(this._dataSources));
        dataSources[projectIndex].dataPath.splice(dataPahtIndex, 1);
        this._dataSources = dataSources;
    }
}
