/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
import { ActiveDataSource, type DataSource, GLOBAL_HOST, LayerType } from '../centralServer/websocket/defs';
import type { CompareData } from '@/utils/Compare';
import { SessionAction } from '@/utils/enum';
import { deleteProjectDataPath } from '@/utils/Project';

// Scene：数据场景：默认、集群、算子调优、Jupter、只trace.json文件
export type Scene = 'Default' | 'Cluster' | 'Compute' | 'Jupyter' | 'OnlyTraceJson';

interface ContextMenu {
    visible: boolean;
}
export interface File {
    projectName: string;
    fileType: LayerType;
    filePath: string;
    rankId?: string;
}

export interface Rank extends File {
    rankId: string;
    cardName: string;
    cardPath: string;
    host?: string;
}

export const DEFAULT_ACTIVE_DATASOURCE: ActiveDataSource = { ...GLOBAL_HOST, projectName: '', projectPath: [], children: [], selectedFileType: 'UNKNOWN', selectedFilePath: '' };

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
    isReset: boolean = false;
    isFullDb: boolean = false;
    isOnlyTraceJson: boolean = false;
    hasCachelineRecords: boolean = false;
    instrVersion: number = -1;
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
    selectedFile: File = { projectName: '', fileType: 'UNKNOWN', filePath: '' };
    compareSet: {baseline: CompareData;comparison: CompareData} = {
        baseline: { projectName: '', fileType: 'UNKNOWN', filePath: '', rankId: '' },
        comparison: { projectName: '', fileType: 'UNKNOWN', filePath: '', rankId: '' },
    };

    // 需要下发给插件的url
    toIframeUrl: string = '';

    // 资源目录的编辑状态
    projectContentEditStatus: boolean = false;

    // 数据源/项目管理
    private _dataSources: DataSource[] = [];

    private _activeDataSource: ActiveDataSource = DEFAULT_ACTIVE_DATASOURCE;

    constructor() {
        makeAutoObservable(this);
    }

    // 导入数据场景：默认、集群、算子调优、Jupter、只trace.json
    get scene(): Scene {
        let scene: Scene;
        if (this.isOnlyTraceJson) {
            scene = 'OnlyTraceJson';
        } else if (this.isBinary) {
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

    get activeDataSource(): ActiveDataSource {
        return this._activeDataSource;
    }

    set dataSources(data: DataSource[]) {
        this._dataSources = data;
    }

    set activeDataSource(data: ActiveDataSource) {
        this._activeDataSource = data;
    }

    // remove：删除工程，不改变页签
    reset(remove?: boolean): void {
        this.isIpynb = false;
        this.isCluster = remove ? false : null;
        this.isBinary = remove ? false : null;
        this.hasCachelineRecords = false;
        this.toIframeUrl = '';
        this.startTime = -1;
        this.endTimeAll = -1;
        this.parseCompleted = false;
        this.clusterCompleted = false;
        this.durationFileCompleted = false;
        this.unitcount = 0;
        this.coreList = [];
        this.sourceList = [];
        this.memoryRankIds = [];
        this.operatorRankIds = [];
    }

    // 数据源管理
    deleteDataSource(projectIndex: number): void {
        this._dataSources = this._dataSources.filter((dataSource, index) => index !== projectIndex);
    }

    deleteDataPath(projectIndex: number, dataPath: string): void {
        const dataSources = JSON.parse(JSON.stringify(this._dataSources));
        deleteProjectDataPath(dataSources[projectIndex], dataPath);
        this._dataSources = dataSources;
    }
}
