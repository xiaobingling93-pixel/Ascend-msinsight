/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
import {
    ActiveDataSource,
    type DataSource,
    FileOrDirectory,
    GLOBAL_HOST,
    LayerType,
} from '@/centralServer/websocket/defs';
import type { CompareData } from '@/utils/Compare';
import { SessionAction } from '@/utils/enum';
import { deleteProjectDataPath } from '@/utils/Project';

// Scene：数据场景：默认、集群、算子调优、Jupter、Leaks、只trace.json文件
export type Scene = 'Default' | 'Cluster' | 'Compute' | 'Jupyter' | 'OnlyTraceJson' | 'IE' | 'Leaks';

interface ContextMenu {
    visible: boolean;
}
export interface File {
    projectName: string;
    fileType: LayerType;
    filePath: string;
    rankId?: string;
}

export interface ClusterFile extends File {
    baselineClusterPath: string;
    currentClusterPath: string;
}

export interface Rank extends File {
    rankId: string;
    cardName: string;
    cardPath: string;
    host?: string;
}

interface ClusterInfo {
    name: string;
    path: string;
    parsed: boolean;
    durationParsed: boolean;
}

export interface ClusterPageInfo {
    clusterList: ClusterInfo[];
    selectedClusterPath: string;
}

export interface TimelinePageInfo {
    unitCount: number;
}

export interface CardInfo {
    cardId: string;
    dbPath: string;
    index?: number;
}

export interface RankInfo {
    clusterId: string;
    host: string;
    rankName: string;
    rankId: string;
    deviceId: string;
}

export interface CardRankInfo {
    rankInfo: RankInfo;
    dbPath: string;
    index?: number;
}

export const DEFAULT_ACTIVE_DATASOURCE: ActiveDataSource = { ...GLOBAL_HOST, projectName: '', projectPath: [], children: [], selectedFileType: 'UNKNOWN', selectedFilePath: '' };

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    defaultConnected?: boolean;
    actionListener: { type: SessionAction; value: string } = { type: SessionAction.NO_ACTION, value: '' };
    // 加载状态
    loading: boolean = false;
    // 数据源/项目管理
    rankMap: Map<string, Rank> = new Map();
    // 场景
    isCluster: boolean | null = false;
    isBinary: boolean | null = false;
    isIpynb: boolean = false;
    isIE: boolean | null = false;
    ipynbUrl: string = '';
    isReset: boolean = false;
    isFullDb: boolean = false;
    isOnlyTraceJson: boolean = false;
    isLeaks: boolean = false;
    hasCachelineRecords: boolean = false;
    instrVersion: number = -1;
    // 解析状态
    parseCompleted: boolean = false;
    clusterPageInfo: ClusterPageInfo = { clusterList: [], selectedClusterPath: '' };
    timelinePageInfo: TimelinePageInfo = { unitCount: 0 };
    // 模块数据
    startTime: number = -1;
    endTimeAll: number = -1;
    unitcount: number = 0;
    memoryCardInfos: Array<Required<CardRankInfo>> = [];
    operatorCardInfos: Array<Required<CardRankInfo>> = [];
    iERankIds: string[] = [];
    deviceIds: any = {};
    threadIds: number[] = [];
    // 模块数据-算子调优
    coreList: string[] = [];
    sourceList: string[] = [];
    // 右键菜单
    contextMenu: ContextMenu = { visible: false };
    // 对比功能
    selectedFile: File = { projectName: '', fileType: 'UNKNOWN', filePath: '' };
    compareSet: { baseline: CompareData; comparison: CompareData } = {
        baseline: { projectName: '', fileType: 'UNKNOWN', filePath: '', rankId: '' },
        comparison: { projectName: '', fileType: 'UNKNOWN', filePath: '', rankId: '' },
    };

    // 需要下发给插件的url
    toIframeUrl: string = '';

    // 资源目录的编辑状态
    projectContentEditStatus: boolean = false;
    // [summary] MoE 热力图数据解析结果
    profilingExpertDataParsed: boolean | null = null;

    // 数据源/项目管理
    private _dataSources: DataSource[] = [];
    private _dataSourceIndexMap: Map<string, number> = new Map();

    private _activeDataSource: ActiveDataSource = DEFAULT_ACTIVE_DATASOURCE;

    constructor() {
        makeAutoObservable(this);
    }

    // 导入数据场景：默认、集群、算子调优、Jupter、只trace.json
    get scene(): Scene {
        let scene: Scene;
        if (this.isOnlyTraceJson) {
            scene = 'OnlyTraceJson';
        } else if (this.isLeaks) {
            scene = 'Leaks';
        } else if (this.isBinary) {
            scene = 'Compute';
        } else if (this.isCluster) {
            scene = 'Cluster';
        } else if (this.isIpynb) {
            scene = 'Jupyter';
        } else if (this.isIE) {
            scene = 'IE';
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
        this._dataSourceIndexMap = new Map();
        data.forEach((item, index): void => {
            this._dataSourceIndexMap.set(item.projectName, index);
        });
    }

    set activeDataSource(data: ActiveDataSource) {
        this._activeDataSource = data;
        this.updateClusterPageInfo(data);
    }

    getDataSourceByProjectName(name: string): DataSource | undefined {
        const idx = this._dataSourceIndexMap.get(name);
        if (idx === undefined) {
            return undefined;
        }
        return this._dataSources[idx];
    }

    setClusterParsed(clusterPath: string): void {
        const currentClusterList = [...this.clusterPageInfo.clusterList];
        const foundIdx = currentClusterList.findIndex(({ path }) => path === clusterPath);
        if (foundIdx >= 0) {
            currentClusterList[foundIdx].parsed = true;
        }
        this.clusterPageInfo.clusterList = currentClusterList;
    }

    setClusterDurationParsed(clusterPath: string): void {
        const currentClusterList = [...this.clusterPageInfo.clusterList];
        const foundIdx = currentClusterList.findIndex(({ path }) => path === clusterPath);
        if (foundIdx >= 0) {
            currentClusterList[foundIdx].durationParsed = true;
        }
        this.clusterPageInfo.clusterList = currentClusterList;
    }

    // remove：删除工程，不改变页签
    reset(remove?: boolean): void {
        this.isIpynb = false;
        this.isCluster = remove ? false : null;
        this.isBinary = remove ? false : null;
        this.isIE = remove ? false : null;
        this.hasCachelineRecords = false;
        this.toIframeUrl = '';
        this.startTime = -1;
        this.endTimeAll = -1;
        this.parseCompleted = false;
        this.clusterPageInfo = { clusterList: [], selectedClusterPath: '' };
        this.timelinePageInfo = { unitCount: 0 };
        this.unitcount = 0;
        this.coreList = [];
        this.sourceList = [];
        this.memoryCardInfos = [];
        this.operatorCardInfos = [];
        this.compareSet = {
            baseline: { projectName: '', fileType: 'UNKNOWN', filePath: '', rankId: '' },
            comparison: { projectName: '', fileType: 'UNKNOWN', filePath: '', rankId: '' },
        };
        this.profilingExpertDataParsed = null;
    }

    // 数据源管理
    deleteDataSource(projectIndex: number): void {
        this.dataSources = this._dataSources.filter((dataSource, index) => index !== projectIndex);
    }

    deleteDataPath(projectIndex: number, dataPath: string): DataSource {
        const dataSources = JSON.parse(JSON.stringify(this._dataSources));
        deleteProjectDataPath(dataSources[projectIndex], dataPath);
        this.dataSources = dataSources;
        return dataSources[projectIndex];
    }

    /**
     * 获取 cluster 类型的 path
     * @param selectedProject 选中的工程项目
     */
    getClusterPath(selectedProject: ActiveDataSource): string {
        const selectedFilePath = selectedProject.selectedFilePath;
        let clusterPath: string = '';
        const findClusterPath = (children: FileOrDirectory[], depth: number): string => {
            if (depth >= 5) { return ''; }
            for (let i = 0; i < children.length; ++i) {
                const item = children[i];
                if (item.type === 'CLUSTER') {
                    clusterPath = item.path;
                }
                if (item.path === selectedFilePath) {
                    return clusterPath;
                }
                const childFindPath = findClusterPath(item.children, depth + 1);
                if (childFindPath !== '') {
                    return childFindPath;
                }
                if (item.type === 'CLUSTER') {
                    clusterPath = '';
                }
            }
            return '';
        };
        return findClusterPath(selectedProject.children, 0);
    }

    private getClusterInfoByPath(path: string): ClusterInfo | undefined {
        return this.clusterPageInfo.clusterList.find((item) => item.path === path);
    }

    private updateClusterPageInfo(selectedProject: ActiveDataSource): void {
        let clusterList = selectedProject.children.filter((child) => child.type === 'CLUSTER').map((child) => {
            const prev = this.getClusterInfoByPath(child.path);
            return {
                name: child.name,
                path: child.path,
                parsed: prev?.parsed ?? false,
                durationParsed: prev?.durationParsed ?? false,
            } as ClusterInfo;
        });
        let selectedClusterPath: string = this.getClusterPath(selectedProject);
        if (clusterList.length === 0) {
            const prev = this.getClusterInfoByPath(selectedProject.projectPath[0]);
            clusterList = [{
                name: selectedProject.projectName,
                path: selectedProject.projectPath[0],
                parsed: prev?.parsed ?? false,
                durationParsed: prev?.durationParsed ?? false,
            }];
            selectedClusterPath = selectedProject.projectPath[0];
        }
        this.clusterPageInfo.clusterList = clusterList;
        this.clusterPageInfo.selectedClusterPath = selectedClusterPath;
    }
}
