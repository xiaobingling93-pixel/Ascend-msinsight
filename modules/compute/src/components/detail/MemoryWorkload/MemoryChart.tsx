/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { drawFlowChart } from './FlowChart/draw';
import * as d3 from 'd3';
import { type Icondition } from './Filter';
import { queryMemoryGraph } from '../../RequestUtils';
import { type Session } from '../../../entity/session';
import { Hit } from 'ascend-utils';
import { CompareData } from '../../../utils/interface';
export interface ImemoryData {
    blockId: string;
    blockType: string;
    chipType: string;
    memoryUnit: Array<CompareData<ImemoryUnit>>;
    l2Cache: CompareData<L2Cache>;
    vector?: CompareData<Icore>;
    vector1?: CompareData<Icore>;
    cube?: CompareData<Icore>;
    advice: string[];
}

export interface L2Cache {
    hit: string;
    miss: string;
    totalRequest: string;
    hitRatio: string;
}

export interface Icore {
    cycle: string;
    totalCycles: string;
    ratio: string;
}
export interface ImemoryUnit {
    [prop: string]: string;
    memoryPath: string;
    request: string;
    requestPerByte: string;
    peakRatio: string;
}

const defaultData = {
    blockId: '',
    blockType: '',
    chipType: '',
    memoryUnit: [],
    l2Cache: {
        compare: {
            hit: '',
            miss: '',
            totalRequest: '',
            hitRatio: '',
        },
        baseline: {
            hit: '',
            miss: '',
            totalRequest: '',
            hitRatio: '',
        },
        diff: {
            hit: '',
            miss: '',
            totalRequest: '',
            hitRatio: '',
        },
    },
    advice: [],
};

const chartId = 'memory';
const chart = observer(({ condition, session }: {condition: Icondition;session: Session}): JSX.Element => {
    const [data, setData] = useState<ImemoryData>(defaultData);
    const [style, setStyle] = useState({ height: '420px' });
    const { t: tDetails } = useTranslation('details');
    const updateData = async (): Promise<void> => {
        const res = await queryMemoryGraph(condition);
        const newData = (res?.coreMemory?.[0] ?? defaultData) as ImemoryData;
        setData(newData);
    };

    useEffect(() => {
        if (!session.parseStatus) {
            setTimeout(() => {
                if (!session.parseStatus) {
                    setData(defaultData);
                }
            }, 200);
            return;
        }
        updateData();
    }, [condition, session.parseStatus]);
    useEffect(() => {
        const svg = d3.select(`#${chartId}>svg`);
        drawFlowChart(svg, { ...data, ...condition, theme: session.theme }, tDetails);
    }, [data, condition, session.theme, tDetails]);
    useEffect(() => {
        let newStyle;
        const { blockType, chipType } = data;
        const chartType = blockType + chipType.slice(0, 3);
        switch (chartType) {
            case 'mix910':
                newStyle = { height: '670px' };
                break;
            default:
                newStyle = { height: '420px' };
                break;
        }
        setStyle(newStyle);
    }, [data]);
    return <div>
        <div id={chartId} style={{ ...style, width: '1220px', margin: '10px auto' }}>
            <svg width={'100%'} height={'100%'}></svg>
        </div>
        { data.advice?.length > 0 && (<Hit text={data.advice} />) }
    </div>;
});

export default chart;
