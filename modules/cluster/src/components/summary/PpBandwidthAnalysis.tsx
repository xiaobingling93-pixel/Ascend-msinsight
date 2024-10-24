/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react-lite';
import type { Session } from '../../entity/session';
import { Empty } from 'ascend-components';
import {
    chartVisbilityListener,
    COLOR,
    commonEchartsOptions,
    notNullObj,
} from '../Common';
import React, { useEffect, useState } from 'react';
import * as echarts from 'echarts';
import Filter, { type ConditionDataType } from './PpBandwidthFilter';
import type { CategoryAxisBaseOption } from 'echarts/types/src/coord/axisCommonTypes';
import ReactDOM from 'react-dom';
import _ from 'lodash';
import styled from '@emotion/styled';
import { chartColors, disposeAdaptiveEchart, getAdaptiveEchart, getDefaultChartOptions } from 'ascend-utils';
import { useTheme } from '@emotion/react';
import { useTranslation } from 'react-i18next';

const ChartsContainer = styled.div`
  display: flex;
  align-items: center;
  gap: 24px;

  .chart-item{
    flex: 1;
    padding: 16px 24px;
    border: 1px solid ${(props): string => props.theme.borderColor};
  }
`;

const PpBandwidthAnalysis = observer(({ session }: { session: Session }) => {
    const [allStageIds, setAllStageIds] = useState<string[]>([]);
    const [conditions, setConditions] = useState<ConditionDataType>(
        { step: '', stage: '' });

    return (
        <div>
            <Filter session={session} setAllStageIds={setAllStageIds} conditions={conditions} setConditions={setConditions}/>
            <PPBandwidthChart conditions={conditions} allStageIds={allStageIds} session={session}/>
        </div>
    );
});

const PPBandwidthChart: React.FC<any> = ({ conditions, allStageIds, session }: {conditions: ConditionDataType; allStageIds: string[];session: Session}) => {
    const theme = useTheme();
    const { i18n } = useTranslation();
    const isDark = theme.mode === 'dark';
    const locale = i18n.language?.slice(0, 2);
    const { step: stepId, stage } = conditions;

    function init(): void {
        if (session.clusterCompleted && notNullObj(conditions)) {
            InitCharts({ domId: 'STAGE', stepId, stage, allStageIds, isDark, locale });
            InitCharts({ domId: 'RANK', stepId, stage, allStageIds, isDark, locale });
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
    }, [conditions, allStageIds, theme, locale]);
    return (
        <ChartsContainer>
            <div className={'chart-item'}>
                <div id={'STAGE'} style={{ height: '600px', width: '100%' }}/>
            </div>
            <div className={'chart-item'}>
                <div id={'RANK'} style={{ height: '600px', width: '100%' }}/>
            </div>
        </ChartsContainer>
    );
};

function renderEmpty(domId: string): void {
    const chartDom = document.getElementById(domId);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    disposeAdaptiveEchart(chartDom);
    ReactDOM.render((<Empty image={Empty.PRESENTED_IMAGE_SIMPLE}/>), chartDom);
}

interface InitChartsParams {
    domId: string;
    stepId: string;
    stage: string;
    allStageIds: string[];
    isDark: boolean;
    locale: string;
}

async function InitCharts({ domId, stepId, stage, allStageIds, isDark, locale }: InitChartsParams): Promise<void> {
    const chartDom = document.getElementById(domId);
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    try {
        const stageIds = stage !== 'All' ? [stage] : allStageIds;
        const res = domId === 'STAGE' ? await wrapBandwidthDataInStage(domId, stepId, stageIds, isDark) : await wrapBandwidthDataInRank(domId, stepId, stageIds, isDark);
        if (res === null || res === undefined) {
            ReactDOM.render((<Empty image={Empty.PRESENTED_IMAGE_SIMPLE}/>), chartDom);
        } else {
            disposeAdaptiveEchart(chartDom);
            const myChart = getAdaptiveEchart(chartDom, null, { locale });
            myChart.setOption(res, true);
            myChart.dispatchAction({
                type: 'takeGlobalCursor',
                key: 'dataZoomSelect',
                dataZoomSelectActive: true,
            });
        }
    } catch (error) {
        disposeAdaptiveEchart(chartDom);
        ReactDOM.render((<></>), chartDom);
    }
}

async function wrapBandwidthDataInStage(domId: string, stepId: string, stageIds: string[], isDark: boolean): Promise<echarts.EChartsOption | null> {
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
    _.merge(result.toolbox, getDefaultChartOptions(isDark).toolbox);

    return result;
}

async function wrapBandwidthDataInRank(domId: string, stepId: string, stageIds: string[], isDark: boolean): Promise<echarts.EChartsOption | null> {
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
    _.merge(result.toolbox, getDefaultChartOptions(isDark).toolbox);

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

async function getStageAndBubbleTimeData(stepId: string, stageId: string): Promise<any> {
    const stageAndBubbleTimeList = await window.requestData('parallelism/pipeline/stageAndBubbleTime',
        { stepId, stageId }, 'summary');
    return stageAndBubbleTimeList?.stageAndBubbleTimes ?? [];
}

async function getRankAndBubbleTimeData(stepId: string, stageId: string): Promise<any> {
    const RankAndBubbleTimeData = await window.requestData('parallelism/pipeline/rankAndBubbleTime', { stepId, stageId }, 'summary');
    return RankAndBubbleTimeData?.stageAndBubbleTimes ?? [];
}

const bandwidthOption: echarts.EChartsOption = {
    textStyle: getDefaultChartOptions().textStyle,
    color: chartColors,
    tooltip: commonEchartsOptions.tooltip,
    toolbox: {
        feature: {
            dataView: { show: true },
            dataZoom: {
                yAxisIndex: 'none',
            },
            restore: { show: true },
        },
    },
    legend: {
        data: [
            { name: 'Stage Time', textStyle: { color: COLOR.GREY_50 } },
            { name: 'Bubble Time', textStyle: { color: COLOR.GREY_50 } },
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
                color: COLOR.GREY_40,
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
                color: COLOR.GREY_40,
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
