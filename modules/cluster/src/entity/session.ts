/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { makeAutoObservable } from 'mobx';
import {
    Communicator,
    communicatorContainerData,
    RankDyeingData,
} from '../components/communicatorContainer/ContainerUtils';
import { IndicatorsItem, PerformanceDataItem } from '../utils/interface';

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
    performanceData: PerformanceDataItem[] = []; // 性能数据
    communicationDomains: string[] = []; // 通信域（包含所有连线、框）
    stepList: string[] = [];

    constructor(conf?: Partial<Session>) {
        makeAutoObservable(this);

        window.closeWaiting = (): void => {
            this.clusterCompleted = true;
            this.parseCompleted = true;
        };
    }

    get dataTypeOptions(): IndicatorsItem[] {
        return this.indicatorList.filter(item => item.renderHeatMap);
    }

    get indicatorMap(): Map<string, IndicatorsItem> {
        const map: Map<string, IndicatorsItem> = new Map();
        this.indicatorList.forEach(item => {
            map.set(item.key, item);
        });
        return map;
    }

    get performanceDataMap(): PerformanceDataMap {
        const map: Map<number, PerformanceDataItem> = new Map();
        this.performanceData.forEach(item => {
            map.set(item.index, item);
        });
        return map;
    }

    // 着色数据图例
    get rankDyeingData(): RankDyeingData {
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
        });

        return data;
    }
}
