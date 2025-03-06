/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { makeAutoObservable } from 'mobx';
import {
    Communicator,
    communicatorContainerData,
    RankDyeingData,
} from '../components/communicatorContainer/ContainerUtils';
import { ClickOperatorItem, IndicatorsItem, PerformanceDataItem } from '../utils/interface';

export type PerformanceDataMap = Map<number, PerformanceDataItem>;

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    parseCompleted: boolean = false;
    clusterCompleted: boolean = false;
    durationFileCompleted: boolean = false;
    unitcount: number = 0;
    renderId: number = 1;
    isFullDb: boolean = false;
    id = '';

    allRankIds: any[] = [];
    communicatorData: communicatorContainerData = {
        partitionModes: [],
        defaultPPSize: 0,
    };

    activeCommunicator: Communicator | undefined;
    rankCount: number = 0;
    arrangementRankCount: number = 0;
    indicatorList: IndicatorsItem[] = []; // 性能指标项列表
    dynamicsIndicatorList: IndicatorsItem[] = []; // 动态性能指标列表
    performanceData: PerformanceDataItem[] = []; // 性能数据
    performanceDataMap: PerformanceDataMap = new Map(); // 性能数据map
    rankDyeingData: RankDyeingData = {}; // 着色图例数据
    communicationDomains: string[] = []; // 通信域（包含所有连线、框）
    ppCommunicationDomains: string[] = []; // PP 通信域（仅包含所有 PP 连线）
    stepList: string[] = [];
    baselineStepList: string[] = [];
    // 集群对比
    isCompare: boolean = false;
    // 右键选中的算子
    targetOperator: ClickOperatorItem | undefined = undefined;

    constructor(conf?: Partial<Session>) {
        makeAutoObservable(this);

        window.closeWaiting = (): void => {
            this.clusterCompleted = true;
            this.parseCompleted = true;
        };
    }

    // Data Type 指标选项（着色指标）
    get dataTypeOptions(): IndicatorsItem[] {
        return this.indicatorList.filter(item => item.renderHeatMap);
    }

    // Order By 指标选项（性能图表指标）
    get performanceChartsIndicators(): IndicatorsItem[] {
        return this.indicatorList.filter(item => item.renderChart);
    }

    get indicatorMap(): Map<string, IndicatorsItem> {
        const map: Map<string, IndicatorsItem> = new Map();
        this.indicatorList.forEach(item => {
            map.set(item.key, item);
        });
        return map;
    }

    // 设置着色数据图例
    setRankDyeingData(): void {
        const data: Record<string, { min: number; max: number }> = {};
        this.dataTypeOptions.forEach(dataType => {
            data[dataType.key] = { min: Number.MAX_SAFE_INTEGER, max: 0 };
        });
        this.performanceData.forEach(item => {
            this.dataTypeOptions.forEach(dataType => {
                const key = dataType.key;
                data[key].max = Math.max(data[key].max, item[key]);
                data[key].min = Math.min(data[key].min, item[key]);
            });
            Object.keys(item.commCompare).forEach(key => {
                if (!(key in data)) {
                    data[key] = { min: Number.MAX_SAFE_INTEGER, max: 0 };
                }
                data[key].max = Math.max(data[key].max, item.commCompare[key]);
                data[key].min = Math.min(data[key].min, item.commCompare[key]);
            });
        });

        this.rankDyeingData = data;
    }

    reset(): void {
        this.clusterCompleted = false;
        this.parseCompleted = false;
        this.unitcount = 0;
        this.durationFileCompleted = false;
        this.allRankIds = [];
        this.communicatorData = { partitionModes: [], defaultPPSize: 0 };
        this.activeCommunicator = undefined;
        this.indicatorList = [];
        this.performanceData = [];
        this.performanceDataMap = new Map();
        this.communicationDomains = [];
        this.ppCommunicationDomains = [];
        this.stepList = [];
        this.rankDyeingData = {};
        this.arrangementRankCount = 0;
        this.targetOperator = undefined;
    }
}
