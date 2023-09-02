/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react-lite';
import { Session } from '../../entity/session';
import { Col, Layout, Row, Select } from 'antd';
import { addResizeEvent, COLOR, Container, isNull, notNullObj } from '../Common';
import React, { useEffect, useState } from 'react';
import * as echarts from 'echarts';
import Filter, { ConditionDataType } from './PpBandwidthFilter';
import type { CategoryAxisBaseOption } from 'echarts/types/src/coord/axisCommonTypes';

const PpBandwidthAnalysis = observer(function ({ session }: { session: Session }) {
    const [ conditions, setConditions ] = useState<ConditionDataType>(
        { step: '', stage: '' });

    return (
        <Layout>
            <Container
                content={ <Filter handleFilterChange={(value: any) => {
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
    const res = domId === 'STAGE' ? await wrapBandwidthDataInStage(domId, stepId) : await wrapBandwidthDataInRank(domId, stepId, stageId);
    const myChart = echarts.init(chartDom);
    myChart.setOption(bandwidthOption);
    addResizeEvent(myChart);
}

async function wrapBandwidthDataInStage(domId: string, stepId: string): Promise<echarts.EChartsOption | null> {
    const datas = await getStageAndBubbleTimeData(stepId);
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
    const rankData: number[] = [];
    const stageTimeData: number[] = [];
    const bubbleTimeData: number[] = [];
    for (const item of datas) {
        rankData.push(Number(item.rankId));
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
    if (isNull(window.request)) {
        return [ '0', '1', '2', '3' ];
    }
    const steps = await window.request('parallelism/pipeline/getAllSteps', {});
    return steps.data;
};

export const getStagesData = async (param: {stepId: string}): Promise<string[]> => {
    if (isNull(window.request)) {
        return ['(0, 1, 2, 3)'];
    }
    const stages = await window.request('parallelism/pipeline/getAllStages', param);
    return stages.data;
};

async function getStageAndBubbleTimeData (stepId: string): Promise<any> {
    if (isNull(window.request)) {
        return [];
    }
    const stageAndBubbleTimeList = await window.request('parallelism/pipeline/stageAndBubbleTime', { stepId });
    return stageAndBubbleTimeList.stageAndBubbleTimes;
}

async function getRankAndBubbleTimeData (stepId: string, stageId: string): Promise<any> {
    if (isNull(window.request)) {
        return [];
    }
    const RankAndBubbleTimeData = await window.request('parallelism/pipeline/rankAndBubbleTime', { stepId, stageId });
    return RankAndBubbleTimeData.stageAndBubbleTimes;
}

const bandwidthOption: echarts.EChartsOption = {
    tooltip: {
        trigger: 'axis',
        axisPointer: {
            type: 'cross',
            crossStyle: {
                color: '#999',
            },
        },
    },
    toolbox: {
        feature: {
            dataView: { show: true, readOnly: false },
            magicType: { show: true, type: [ 'line', 'bar' ] },
            restore: { show: true },
            saveAsImage: { show: true },
        },
    },
    legend: {
        data: [ 'Stage Time', 'Bubble Time' ],
    },
    xAxis: [
        {
            type: 'category',
            data: [],
            axisPointer: {
                type: 'shadow',
            },
        },
    ],
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
