/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { drawFlowChart } from './FlowChart/draw';
import * as d3 from 'd3';
import { type Icondition } from './Filter';
import { queryMemoryGraph } from '../../RequestUtils';

export interface ImemoryData {
    blockId: string;
    blockType: string;
    chipType: string;
    memoryUnit: ImemoryUnit[] ;
    L2catch: {
        hit: string;
        miss: string;
        totalRequest: string;
        hitRatio: string;
    };
}
export interface ImemoryUnit {
    [prop: string]: string;
    memoryPath: string;
    request: string;
    requestPerByte: string;
}

const defaultData = {
    blockId: '',
    blockType: '',
    chipType: '',
    memoryUnit: [],
    L2catch: {
        hit: '',
        miss: '',
        totalRequest: '',
        hitRatio: '',
    },
};

const chartId = 'memory';
const chart = observer(({ condition }: {condition: Icondition}): JSX.Element => {
    const [data, setData] = useState<ImemoryData>(defaultData);
    const updateData = async (): Promise<void> => {
        const res = await queryMemoryGraph(condition);
        const newData = (res?.coreMemory?.[0] ?? defaultData) as ImemoryData;
        setData(newData);
    };

    useEffect(() => {
        updateData();
    }, [condition.blockId]);
    useEffect(() => {
        const svg = d3.select(`#${chartId}>svg`);
        drawFlowChart(svg, { ...data, ...condition });
    }, [data, condition.showAs]);
    return <div id={chartId} style={{ height: '670px', width: '980px', margin: '10px auto' }}>
        <svg width={'100%'} height={'100%'}></svg>
    </div>;
});

export default chart;
