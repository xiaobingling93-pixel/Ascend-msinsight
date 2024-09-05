/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import _ from 'lodash';
import { notZero } from '../Common';

export interface communicatorContainerData {
    partitionModes: partitionMode[];
    defaultPPSize: number;
};

export interface partitionMode {
    mode: string;
    communicators: communicator[];
};

export interface communicator {
    name: string;
    ranks: number[];
    value?: string;
};

export interface tabData {
    label: string;
    key: string;
    children: JSX.Element;
};

interface formFieldsValue {
    ppSize: number;
    tpSize: number;
    dpSize: number;
    algorithm: string;
};

interface fillCommunicatorsType {
    values: formFieldsValue;
    partitionModes: partitionMode[];
    pipelineSize: number;
    rankNum?: number;
    modelCount?: number;
};

export const titleMap = new Map([
    ['pp', 'Pipeline Parallel'],
    ['tp', 'Tensor Parallel'],
    ['dp', 'Data Parallel'],
    ['tpOrDp', 'Tensor/Data Parallel'],
]);

export const getPpContainerData = (data: communicatorContainerData, mode: string): any[] => {
    const tmp = data.partitionModes?.find(vl => vl.mode === mode);
    if (tmp === undefined) {
        return [];
    }
    const result = [];
    result.push({ label: 'All', value: 'All' });
    tmp?.communicators.forEach(item => {
        result.push({ value: item.value, label: item.value });
    });
    return result;
};

export const getAllPpStageIds = (data: communicatorContainerData): string[] => {
    const tmp = data.partitionModes?.find(vl => vl.mode === 'pp');
    if (tmp === undefined) {
        return [];
    }
    return tmp?.communicators.map(item => item.value as string);
};

const fillPpCommunicators = ({ values, rankNum, pipelineSize, partitionModes }: fillCommunicatorsType): void => {
    if (values.ppSize < 1) {
        return;
    }
    const communicators = getCommunicators(partitionModes, 'pp');
    let ranks: number[] = [];
    for (let i = 0; i < pipelineSize; i++) {
        switch (values.algorithm) {
            case 'Megatron-LM(tp-dp-pp)':
                ranks = _.range(i, rankNum, pipelineSize);
                break;
            case 'Megatron-LM(tp-pp-dp)':
                ranks = _.range(i * values.ppSize, (i + 1) * values.ppSize, 1);
                break;
            default:
                break;
        }
        communicators.push({
            ranks,
            name: `pipeline${i}`,
            value: `(${ranks.join(', ')}${pipelineSize > 1 ? ')' : ',)'}`,
        });
    }
};

const fillTpCommunicators = ({ values, modelCount, partitionModes }: fillCommunicatorsType): void => {
    if (values.tpSize < 1 || modelCount === undefined) {
        return;
    }
    const communicators = getCommunicators(partitionModes, 'tp');
    let ranks: number[] = [];
    let start = 0;
    for (let i = 0; i < modelCount; i++) {
        switch (values.algorithm) {
            case 'Megatron-LM(tp-dp-pp)':
                ranks = _.range(i * values.tpSize, (i + 1) * values.tpSize);
                break;
            case 'Megatron-LM(tp-pp-dp)':
                start = Math.floor(i / values.dpSize) + ((i % values.dpSize) * (values.ppSize * values.tpSize));
                ranks = _.range(start, start + (values.ppSize * values.tpSize), values.ppSize);
                break;
            default:
                break;
        }
        communicators.push({
            ranks,
            name: `model${i}`,
            value: `(${ranks.join(', ')}${values.tpSize > 1 ? ')' : ',)'}`,
        });
    }
};

const fillDpCommunicators = ({ values, rankNum, pipelineSize, partitionModes }: fillCommunicatorsType): void => {
    if (values.dpSize < 1 || rankNum === undefined) {
        return;
    }
    const communicators = getCommunicators(partitionModes, 'dp');
    let ranks: number[] = [];
    for (let i = 0; i < values.ppSize; i++) {
        for (let j = 0; j < values.tpSize; j++) {
            switch (values.algorithm) {
                case 'Megatron-LM(tp-dp-pp)':
                    ranks = _.range((i * pipelineSize) + j, ((i + 1) * pipelineSize) + j, values.tpSize);
                    break;
                case 'Megatron-LM(tp-pp-dp)':
                    ranks = _.range(i + (j * values.ppSize), rankNum - (values.ppSize - i - 1), values.tpSize * values.ppSize);
                    break;
                default:
                    break;
            }
            communicators.push({
                ranks,
                name: `data${(i * values.tpSize) + j}`,
                value: `(${ranks.join(', ')}${values.dpSize > 1 ? ')' : ',)'}`,
            });
        }
    }
};

const fillDpRectCommunicators = ({ values, pipelineSize, partitionModes }: fillCommunicatorsType): void => {
    if (values.dpSize < 1 || values.ppSize < 1) {
        return;
    }
    const communicators = getCommunicators(partitionModes, 'dpRect');
    let ranks: number[] = [];
    for (let i = 0; i < values.dpSize; i++) {
        switch (values.algorithm) {
            case 'Megatron-LM(tp-dp-pp)':
                ranks = [];
                for (let j = 0; j < values.ppSize; j++) {
                    ranks.push(..._.range((j * pipelineSize) + (i * values.tpSize), (j * pipelineSize) + ((i + 1) * values.tpSize), 1));
                }
                break;
            case 'Megatron-LM(tp-pp-dp)':
                ranks = _.range(i * (values.ppSize * values.tpSize), (i + 1) * (values.ppSize * values.tpSize), 1);
                break;
            default:
                break;
        }
        communicators.push({
            ranks,
            name: `dataRect${i}`,
            value: `(${ranks.join(', ')})`,
        });
    }
};

const getCommunicators = (partitionModes: partitionMode[], key: string): communicator[] => {
    return partitionModes.find(item => item.mode === key)?.communicators ?? [];
};

export const generateCommunicatorData = (values: formFieldsValue, rankNum: number, defaultPPSize: number = 1): communicatorContainerData => {
    const partitionModes: partitionMode[] = [
        { mode: 'all', communicators: [] },
        { mode: 'pp', communicators: [] },
        { mode: 'tp', communicators: [] },
        { mode: 'dp', communicators: [] },
        { mode: 'dpRect', communicators: [] },
    ];
    if (rankNum === 0) {
        return { partitionModes, defaultPPSize };
    }
    if (values.ppSize !== 0 && values.tpSize !== 0) {
        const pipelineSize = rankNum / values.ppSize;
        const modelCount = rankNum / values.tpSize;

        partitionModes[0].communicators.push({
            ranks: _.range(0, rankNum, 1),
            name: 'All',
            value: 'All',
        });
        fillPpCommunicators({ values, rankNum, pipelineSize, partitionModes });
        fillTpCommunicators({ values, modelCount, pipelineSize, partitionModes });
        fillDpCommunicators({ values, rankNum, pipelineSize, partitionModes });
        fillDpRectCommunicators({ values, pipelineSize, partitionModes });
    }
    return { partitionModes, defaultPPSize };
};

export interface rankItem {
    value: number;
    site: number[];
};
export interface tpData {
    name: string;
    key: string;
    values: rankItem[];
};

export interface dpData {
    name: string;
    key: string;
    values: tpData[];
};

export interface ppData {
    name: string;
    key: string;
    values: dpData[];
};

export const getRankData = (values: formFieldsValue): ppData[] => {
    const ranksData: ppData[] = [];
    const rankNum = values.ppSize * values.tpSize * values.dpSize;
    if (values.ppSize < 1 || values.tpSize < 1 || values.dpSize < 1) {
        return ranksData;
    }
    const pipelineSize = rankNum / values.ppSize;
    const dataSize = pipelineSize / values.dpSize;
    const tensorSize = dataSize / values.tpSize;
    for (let i = 0; i < values.ppSize; i++) {
        let ppValue: number[] = [];
        switch (values.algorithm) {
            case 'Megatron-LM(tp-dp-pp)':
                ppValue = _.range(pipelineSize * i, pipelineSize * (i + 1), 1);
                break;
            case 'Megatron-LM(tp-pp-dp)':
                ppValue = _.range(i, rankNum, values.ppSize);
                break;
            default:
                break;
        }
        const dpValue: dpData[] = [];
        for (let j = 0; j < values.dpSize; j++) {
            const dpData: number[] = ppValue.slice(dataSize * j, dataSize * (j + 1));
            const tpValue: tpData[] = [];
            for (let n = 0; n < tensorSize; n++) {
                const rank = dpData.slice(values.tpSize * n, values.tpSize * (n + 1)).map((item, index) => ({
                    value: item,
                    site: [i, j, n, index],
                }));
                tpValue.push({
                    name: `tp${n}`,
                    key: `tp${n}`,
                    values: rank,
                });
            }
            dpValue.push({
                name: `DP ${j}`,
                key: `dp${j}`,
                values: tpValue,
            });
        }
        ranksData.push({
            name: `PP Stage ${i}`,
            key: `pp${i}`,
            values: dpValue,
        });
    }
    return ranksData;
};

export interface summaryListItem {
    [key: string]: any;
    communicationNotOverLappedTime: number;
    communicationOverLappedTime: number;
    computingTime: number;
    freeTime: number;
    prepareTime: number;
    rankId: string;
    totalTime: number;
};

interface rankDyeingDataType {
    [key: string]: { average: number; max: number };
};

let rankDyeingData: rankDyeingDataType = {};
let rankDataList: Array<summaryListItem & { pureComputingTime: number }> = [];

export const computeRankDyeingData = (summaryList: summaryListItem[]): void => {
    rankDyeingData = {
        communicationNotOverLappedTime: { average: 0, max: 0 },
        communicationOverLappedTime: { average: 0, max: 0 },
        computingTime: { average: 0, max: 0 },
        freeTime: { average: 0, max: 0 },
        prepareTime: { average: 0, max: 0 },
        pureComputingTime: { average: 0, max: 0 },
    };
    rankDataList = [];
    let pureComputingTime = 0;
    summaryList.forEach(item => {
        if (item.prepareTime < 0) {
            item.prepareTime = 0;
        }
        pureComputingTime = Number((item.computingTime - item.communicationOverLappedTime).toFixed(2));
        rankDataList.push({
            ...item,
            pureComputingTime,
        });
        computeRankDyeingMax(rankDyeingData, item, pureComputingTime);
        computeRankDyeingCount(rankDyeingData, item, pureComputingTime);
    });
    rankDyeingData.communicationNotOverLappedTime.average /= summaryList.length;
    rankDyeingData.communicationOverLappedTime.average /= summaryList.length;
    rankDyeingData.computingTime.average /= summaryList.length;
    rankDyeingData.freeTime.average /= summaryList.length;
    rankDyeingData.prepareTime.average /= summaryList.length;
    rankDyeingData.pureComputingTime.average /= summaryList.length;
};

const computeRankDyeingMax = (dyeingData: rankDyeingDataType, item: summaryListItem, pureComputingTime: number): void => {
    dyeingData.communicationNotOverLappedTime.max = Math.max(
        dyeingData.communicationNotOverLappedTime.max,
        item.communicationNotOverLappedTime,
    );
    dyeingData.communicationOverLappedTime.max = Math.max(
        dyeingData.communicationOverLappedTime.max,
        item.communicationOverLappedTime,
    );
    dyeingData.computingTime.max = Math.max(dyeingData.computingTime.max, item.computingTime);
    dyeingData.freeTime.max = Math.max(dyeingData.freeTime.max, item.freeTime);
    dyeingData.prepareTime.max = Math.max(dyeingData.prepareTime.max, item.prepareTime);
    dyeingData.pureComputingTime.max = Math.max(dyeingData.pureComputingTime.max, pureComputingTime);
};

const computeRankDyeingCount = (dyeingData: rankDyeingDataType, item: summaryListItem, pureComputingTime: number): void => {
    dyeingData.communicationNotOverLappedTime.average += item.communicationNotOverLappedTime;
    dyeingData.communicationOverLappedTime.average += item.communicationOverLappedTime;
    dyeingData.computingTime.average += item.computingTime;
    dyeingData.freeTime.average += item.freeTime;
    dyeingData.prepareTime.average += item.prepareTime;
    dyeingData.pureComputingTime.average += pureComputingTime;
};

export const getRankDyeingData = (): rankDyeingDataType => {
    return rankDyeingData;
};

export const getOpacity = (rankId: number, dyeingMode: string): number => {
    const rankData = rankDataList.find(item => item.rankId === rankId.toString());
    let opacity = 0;
    if (rankData === undefined) {
        return opacity;
    }
    switch (dyeingMode) {
        case 'prepareTime':
            opacity = computeOpacity(rankData.prepareTime, rankDyeingData.prepareTime);
            break;
        case 'computingTime':
            opacity = computeOpacity(rankData.computingTime, rankDyeingData.computingTime);
            break;
        case 'pureComputingTime':
            opacity = computeOpacity(rankData.pureComputingTime, rankDyeingData.pureComputingTime);
            break;
        case 'communicationOverLappedTime':
            opacity = computeOpacity(rankData.pureComputingTime, rankDyeingData.pureComputingTime);
            break;
        case 'communicationNotOverLappedTime':
            opacity = computeOpacity(rankData.communicationNotOverLappedTime, rankDyeingData.communicationNotOverLappedTime);
            break;
        case 'freeTime':
            opacity = computeOpacity(rankData.freeTime, rankDyeingData.freeTime);
            break;
        default:
            opacity = 1;
            break;
    }
    return opacity;
};

export const computeOpacity = (rankDataValue: number, rankDyeingDataItem: { average: number; max: number }): number => {
    return Number((
        (rankDataValue - rankDyeingDataItem.average) / notZero(rankDyeingDataItem.max - rankDyeingDataItem.average)
    ).toFixed(4));
};

export const getRankDataById = (rankId: number, summaryList: summaryListItem[]):
(summaryListItem & { pureComputingTime: number; computeTimeRatio: number; communicationTimeRatio: number }) | undefined => {
    const rank = summaryList.find(item => item.rankId === rankId.toString());
    if (rank === undefined) {
        return undefined;
    }
    if (rank.prepareTime < 0) {
        rank.prepareTime = 0;
    }
    const totalTime = rank.communicationNotOverLappedTime + rank.computingTime + rank.freeTime + rank.prepareTime;
    const pureComputingTime = Number((rank.computingTime - rank.communicationOverLappedTime).toFixed(2));
    return {
        ...rank,
        pureComputingTime,
        communicationNotOverLappedTime: Number(rank.communicationNotOverLappedTime.toFixed(2)),
        communicationOverLappedTime: Number(rank.communicationNotOverLappedTime.toFixed(2)),
        computingTime: Number(rank.communicationNotOverLappedTime.toFixed(2)),
        freeTime: Number(rank.communicationNotOverLappedTime.toFixed(2)),
        prepareTime: Number(rank.communicationNotOverLappedTime.toFixed(2)),
        computeTimeRatio: Number((rank.computingTime / notZero(totalTime) * 100).toFixed(2)),
        communicationTimeRatio: Number((rank.communicationNotOverLappedTime / notZero(totalTime) * 100).toFixed(2)),
    };
};
