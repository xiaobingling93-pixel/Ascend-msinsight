/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import _ from 'lodash';

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
    tab: string;
    key: string;
    content: JSX.Element;
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

export const generateCommunicatorData = (values: {ppSize: number; tpSize: number; dpSize: number},
    defaultPPSize: number, rankNum: number): communicatorContainerData => {
    const partitionModes: partitionMode[] = [
        { mode: 'pp', communicators: [] },
        { mode: 'tp', communicators: [] },
        { mode: 'dp', communicators: [] },
    ];
    if (values.ppSize !== 0 && values.tpSize !== 0) {
        const pipelineCount = values.ppSize;
        const pipelineSize = rankNum / values.ppSize;
        const modelCount = rankNum / values.tpSize;
        for (let i = 0; i < pipelineCount; i++) {
            partitionModes[0].communicators.push({
                ranks: _.range(i * pipelineSize, (i + 1) * pipelineSize),
                name: `stage${i}`,
                value: `(${_.range(i * pipelineSize, (i + 1) * pipelineSize).join(', ')}${pipelineSize > 1 ? ')' : ',)'}`,
            });
            for (let j = 0; j < values.tpSize; j++) {
                partitionModes[2].communicators.push({
                    ranks: _.range((i * pipelineSize) + j, ((i + 1) * pipelineSize) + j, values.tpSize),
                    name: `data${(i * values.tpSize) + j}`,
                    value: `(${_.range((i * pipelineSize) + j, ((i + 1) * pipelineSize) + j, values.tpSize).join(', ')}${values.dpSize > 1 ? ')' : ',)'}`,
                });
            }
        }
        for (let i = 0; i < modelCount; i++) {
            partitionModes[1].communicators.push({
                ranks: _.range(i * values.tpSize, (i + 1) * values.tpSize),
                name: `model${i}`,
                value: `(${_.range(i * values.tpSize, (i + 1) * values.tpSize).join(', ')}${values.tpSize > 1 ? ')' : ',)'}`,
            });
        }
    }
    return { partitionModes, defaultPPSize };
};
