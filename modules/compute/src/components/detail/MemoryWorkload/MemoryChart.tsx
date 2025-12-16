/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { drawFlowChart } from './FlowChart/draw';
import * as d3 from 'd3';
import { type Icondition } from './Filter';
import { queryMemoryGraph } from '../../RequestUtils';
import { type Session } from '../../../entity/session';
import { CompareData } from '../../../utils/interface';
import { runInAction } from 'mobx';
import { FormulaTip } from './FormulaTip';
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
        if (condition.blockId === '') {
            setData(defaultData);
            return;
        }
        const res = await queryMemoryGraph(condition);
        // 获取echarts数据
        const newData = (res?.coreMemory?.[0] ?? defaultData) as ImemoryData;
        setData(newData);
    };
    //  监听刷新功能
    useEffect(() => {
        if (!session.parseStatus) {
            setData(defaultData);
            return;
        }
        updateData();
    }, [condition, session.parseStatus]);

    useEffect(() => {
        const svg = d3.select(`#${chartId}>svg`);
        // 画图其中的功能
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
        if (chipType === '910A5') {
            switch (blockType) {
                case 'mix':
                    newStyle = { height: '670px' };
                    break;
                default:
                    newStyle = { height: '420px' };
                    break;
            }
        }
        setStyle(newStyle);
    }, [data]);

    useEffect(() => {
        runInAction(() => {
            session.computeAdvice = data.advice;
        });
    }, [data.advice]);
    return <div>
        <div id={chartId} style={{ ...style, width: '1550px', margin: '10px auto' }}>
            <svg width={'100%'} height={'100%'}></svg>
            <FormulaTip session={session}/>
        </div>
    </div>;
});

export default chart;
