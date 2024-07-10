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
    commonEchartsOptions,
    Container,
    notNullObj,
} from '../Common';
import React, { useEffect, useState } from 'react';
import * as echarts from 'echarts';
import Filter, { ConditionDataType } from './PpBandwidthFilter';
import type { CategoryAxisBaseOption } from 'echarts/types/src/coord/axisCommonTypes';
import ReactDOM from 'react-dom';
import _ from 'lodash';

const PpBandwidthAnalysis = observer(({ session }: { session: Session }) => {
    const [allStageIds, setAllStageIds] = useState<string[]>([]);
    const [conditions, setConditions] = useState<ConditionDataType>(
        { step: '', stage: '' });

    return (
        <Layout>
            <Container
                content={ <Filter session={session} setAllStageIds={setAllStageIds} conditions={conditions} setConditions={setConditions}/>}
            />
            <Container
                style={{ minWidth: '630px' }}
                content={ <PPBandwidthChart conditions={conditions} allStageIds={allStageIds} session={session}/>}
            />
        </Layout>
    );
});

const PPBandwidthChart: React.FC<any> = ({ conditions, allStageIds, session }: {conditions: ConditionDataType; allStageIds: string[];session: Session}) => {
    function init(): void {
        if (session.clusterCompleted && notNullObj(conditions)) {
            InitCharts('STAGE', conditions.step, conditions.stage, allStageIds);
            InitCharts('RANK', conditions.step, conditions.stage, allStageIds);
        } else {
            renderEmpty('STAGE');
            renderEmpty('RANK');
        }
    }
    chartVisbilityListener('STAGE', () => {
        init();
    });
    useEffect(() => {
        init();
    }, [conditions, allStageIds]);
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

function renderEmpty(domId: string): void {
    const chartDom = document.getElementById(domId);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    echarts.getInstanceByDom(chartDom)?.dispose();
    ReactDOM.render((<Empty image={Empty.PRESENTED_IMAGE_SIMPLE}/>), chartDom);
}

async function InitCharts(domId: string, stepId: string, stage: string, allStageIds: string[]): Promise<void> {
    const chartDom = document.getElementById(domId);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    try {
        const stageIds = stage !== 'All' ? [stage] : allStageIds;
        const res = domId === 'STAGE' ? await wrapBandwidthDataInStage(domId, stepId, stageIds) : await wrapBandwidthDataInRank(domId, stepId, stageIds);
        if (res === null || res === undefined) {
            ReactDOM.render((<Empty image={Empty.PRESENTED_IMAGE_SIMPLE}/>), chartDom);
        } else {
            const myChart: echarts.ECharts = echarts.getInstanceByDom(chartDom)
                ? echarts.getInstanceByDom(chartDom) as echarts.ECharts
                : echarts.init(chartDom);
            myChart.setOption(res, true);
            myChart.dispatchAction({
                type: 'takeGlobalCursor',
                key: 'dataZoomSelect',
                dataZoomSelectActive: true,
            });
            addResizeEvent(myChart);
        }
    } catch (error) {
        echarts.getInstanceByDom(chartDom)?.dispose();
        ReactDOM.render((<></>), chartDom);
    }
}

async function wrapBandwidthDataInStage(domId: string, stepId: string, stageIds: string[]): Promise<echarts.EChartsOption | null> {
    const result = _.cloneDeep(bandwidthOption);
    const xAxis = result.xAxis as CategoryAxisBaseOption;
    const series = result.series as echarts.SeriesOption[];
    for (const stageId of stageIds) {
        const res = await getStageAndBubbleTimeData(stepId, stageId);
        const datas = res ?? [];
        const stageData: number[] = [];
        const stageTimeData: number[] = [];
        const bubbleTimeData: number[] = [];
        for (const item of datas
            .sort((a: string[], b: string[]) => parseFloat(a[1]) - parseFloat(b[1]))) {
            stageData.push(item.stageId);
            stageTimeData.push(item.stageTime);
            bubbleTimeData.push(item.bubbleTime);
        }
        xAxis.data = xAxis.data?.concat(stageData);
        series[0].data = (series[0].data as number[]).concat(stageTimeData);
        series[1].data = (series[1].data as number[]).concat(bubbleTimeData);
    }
    return result;
}

async function wrapBandwidthDataInRank(domId: string, stepId: string, stageIds: string[]): Promise<echarts.EChartsOption | null> {
    const result = _.cloneDeep(bandwidthOption);
    const xAxis = result.xAxis as CategoryAxisBaseOption;
    const series = result.series as echarts.SeriesOption[];
    for (const stageId of stageIds) {
        const res: RankDataType[] = await getRankAndBubbleTimeData(stepId, stageId);
        const datas = res ?? [];
        datas.sort((a, b) => Number(a.rankId) - Number(b.rankId));
        const rankData: string[] = [];
        const stageTimeData: number[] = [];
        const bubbleTimeData: number[] = [];
        for (const item of datas) {
            rankData.push(item.rankId);
            stageTimeData.push(item.stageTime);
            bubbleTimeData.push(item.bubbleTime);
        }
        xAxis.data = xAxis.data?.concat(rankData);
        series[0].data = (series[0].data as number[]).concat(stageTimeData);
        series[1].data = (series[1].data as number[]).concat(bubbleTimeData);
    }
    return result;
}

export interface RankDataType {
    bubbleTime: number;
    rankId: string;
    stageTime: number ;
}

export const getStepsData = async (): Promise<string[]> => {
    const steps = await window.requestData('parallelism/pipeline/getAllSteps', {}, 'summary');
    return steps?.data ?? [];
};

async function getStageAndBubbleTimeData (stepId: string, stageId: string): Promise<any> {
    const stageAndBubbleTimeList = await window.requestData('parallelism/pipeline/stageAndBubbleTime',
        { stepId, stageId }, 'summary');
    return stageAndBubbleTimeList?.stageAndBubbleTimes ?? [];
}

async function getRankAndBubbleTimeData (stepId: string, stageId: string): Promise<any> {
    const RankAndBubbleTimeData = await window.requestData('parallelism/pipeline/rankAndBubbleTime', { stepId, stageId }, 'summary');
    return RankAndBubbleTimeData?.stageAndBubbleTimes ?? [];
}

const bandwidthOption: echarts.EChartsOption = {
    tooltip: commonEchartsOptions.tooltip,
    toolbox: {
        feature: {
            dataView: { show: true, readOnly: false },
            dataZoom: {
                yAxisIndex: 'none',
            },
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
                width: 300,
                overflow: 'truncate',
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
                valueFormatter: function (value): string {
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
                valueFormatter: function (value): string {
                    return `${value}`;
                },
            },
            data: [],
            barMaxWidth: 80,
        },
    ],
    grid: {
        left: 95,
    },
};

export default PpBandwidthAnalysis;
