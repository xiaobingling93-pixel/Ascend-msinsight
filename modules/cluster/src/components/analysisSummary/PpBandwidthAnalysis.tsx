/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react-lite';
import { Session } from '../../entity/session';
import { Col, Empty, Layout, Row } from 'antd';
import {
    addResizeEvent,
    chartVisbilityListener,
    COLOR,
    Container,
    isNull,
    notNullObj,
} from '../Common';
import React, { useEffect, useState } from 'react';
import * as echarts from 'echarts';
import Filter, { ConditionDataType } from './PpBandwidthFilter';
import type { CategoryAxisBaseOption } from 'echarts/types/src/coord/axisCommonTypes';
import ReactDOM from 'react-dom';

const PpBandwidthAnalysis = observer(function ({ session }: { session: Session }) {
    const [ conditions, setConditions ] = useState<ConditionDataType>(
        { step: '', stage: '' });

    return (
        <Layout>
            <Container
                content={ <Filter session={session} handleFilterChange={(value: any) => {
                    setConditions(value);
                }}/>}
            />
            <Container
                content={ <PPBandwidthChart conditions={conditions}/>}
            />
        </Layout>
    );
});

const PPBandwidthChart: React.FC<any> = ({ conditions }: any) => {
    chartVisbilityListener('STAGE', () => {
        InitCharts('STAGE', conditions.step, conditions.stage);
        InitCharts('RANK', conditions.step, conditions.stage);
    });
    useEffect(() => {
        setTimeout(() => {
            if (notNullObj(conditions)) {
                InitCharts('STAGE', conditions.step, conditions.stage);
                InitCharts('RANK', conditions.step, conditions.stage);
            }
        });
    }, [conditions]);
    return (
        <div className={'bandwidthChart'}>
            <Row wrap={false}>
                <Col span={12}>
                    <div className={'chartDiv'}>
                        <div id={'STAGE'} style={{ height: '600px', width: '100%', display: 'inline-block' }}/>
                    </div>
                </Col>
                <Col span={12}>
                    <div className={'chartDiv'}>
                        <div id={'RANK'} style={{ height: '600px', width: '100%', display: 'inline-block' }}/>
                    </div>
                </Col>
            </Row>
        </div>
    );
};

async function InitCharts(domId: string, stepId: string, stageId: string): Promise<void> {
    const chartDom = document.getElementById(domId);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    const res = domId === 'STAGE' ? await wrapBandwidthDataInStage(domId, stepId, stageId) : await wrapBandwidthDataInRank(domId, stepId, stageId);
    if (res === null) {
        ReactDOM.render((<Empty image={Empty.PRESENTED_IMAGE_SIMPLE}/>), chartDom);
    } else {
        const myChart = echarts.init(chartDom);
        myChart.setOption(res);
        addResizeEvent(myChart);
    }
    const myChart = echarts.init(chartDom);
    myChart.setOption(bandwidthOption);
    addResizeEvent(myChart);
}

async function wrapBandwidthDataInStage(domId: string, stepId: string, stageId: string): Promise<echarts.EChartsOption | null> {
    const datas = await getStageAndBubbleTimeData(stepId, stageId);
    const stageData: number[] = [];
    const stageTimeData: number[] = [];
    const bubbleTimeData: number[] = [];
    for (const item of datas
        .sort((a: string[], b: string[]) => parseFloat(a[1]) - parseFloat(b[1]))) {
        stageData.push(item.stageId);
        stageTimeData.push(item.stageTime);
        bubbleTimeData.push(item.bubbleTime);
    }
    (bandwidthOption.xAxis as CategoryAxisBaseOption).data = stageData;
    (bandwidthOption.series as echarts.SeriesOption[])[0].data = stageTimeData;
    (bandwidthOption.series as echarts.SeriesOption[])[1].data = bubbleTimeData;
    return bandwidthOption;
}

async function wrapBandwidthDataInRank(domId: string, stepId: string, stageId: string): Promise<echarts.EChartsOption | null> {
    const datas: RankDataType[] = await getRankAndBubbleTimeData(stepId, stageId);
    datas.sort((a, b) => Number(a.rankId) - Number(b.rankId));
    const rankData: string[] = [];
    const stageTimeData: number[] = [];
    const bubbleTimeData: number[] = [];
    for (const item of datas) {
        rankData.push(item.rankId);
        stageTimeData.push(item.stageTime);
        bubbleTimeData.push(item.bubbleTime);
    }
    (bandwidthOption.xAxis as CategoryAxisBaseOption).data = rankData;
    (bandwidthOption.series as echarts.SeriesOption[])[0].data = stageTimeData;
    (bandwidthOption.series as echarts.SeriesOption[])[1].data = bubbleTimeData;
    return bandwidthOption;
}

export interface StageDataType {
    bubbleTime: number;
    stageId: string;
    stageTime: number ;
}

export interface RankDataType {
    bubbleTime: number;
    rankId: string;
    stageTime: number ;
}

export const getStepsData = async (): Promise<string[]> => {
    if (isNull(window.requestData)) {
        return [ '0', '1', '2', '3' ];
    }
    const steps = await window.requestData('parallelism/pipeline/getAllSteps', {}, 'summary');
    return steps.data;
};

export const getStagesData = async (param: {stepId: string}): Promise<string[]> => {
    if (isNull(window.requestData)) {
        return ['(0, 1, 2, 3)'];
    }
    const stages = await window.requestData('parallelism/pipeline/getAllStages', param, 'summary');
    return stages.data;
};

async function getStageAndBubbleTimeData (stepId: string, stageId: string): Promise<any> {
    if (isNull(window.requestData)) {
        return [];
    }
    const stageAndBubbleTimeList = await window.requestData('parallelism/pipeline/stageAndBubbleTime',
        { stepId, stageId }, 'summary');
    return stageAndBubbleTimeList.stageAndBubbleTimes;
}

async function getRankAndBubbleTimeData (stepId: string, stageId: string): Promise<any> {
    if (isNull(window.requestData)) {
        return [];
    }
    const RankAndBubbleTimeData = await window.requestData('parallelism/pipeline/rankAndBubbleTime', { stepId, stageId }, 'summary');
    return RankAndBubbleTimeData.stageAndBubbleTimes;
}

const bandwidthOption: echarts.EChartsOption = {
    tooltip: {
        trigger: 'axis',
        axisPointer: {
            type: 'cross',
            crossStyle: {
                color: COLOR.BrightBlue,
                type: 'solid',
            },
        },
    },
    toolbox: {
        feature: {
            dataView: { show: true, readOnly: false },
            restore: { show: true },
        },
    },
    legend: {
        data: [
            { name: 'Stage Time', textStyle: { color: COLOR.Grey50 } },
            { name: 'Bubble Time', textStyle: { color: COLOR.Grey50 } },
        ],
    },
    xAxis:
        {
            type: 'category',
            data: [],
            axisPointer: {
                type: 'shadow',
            },
            axisLabel: {
                color: COLOR.Grey40,
            },
        },
    yAxis: [
        {
            type: 'value',
            name: 'Times(us)',
            axisLabel: {
                formatter: '{value}',
                color: COLOR.Grey40,
            },
        },
    ],
    series: [
        {
            name: 'Stage Time',
            type: 'bar',
            tooltip: {
                valueFormatter: function (value) {
                    return `${value}`;
                },
            },
            data: [],
            barMaxWidth: 80,
        },
        {
            name: 'Bubble Time',
            type: 'bar',
            tooltip: {
                valueFormatter: function (value) {
                    return `${value}`;
                },
            },
            data: [],
            barMaxWidth: 80,
        },
    ],
};

export default PpBandwidthAnalysis;
