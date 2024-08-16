/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/
import React, { useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import * as echarts from 'echarts';
import { observer } from 'mobx-react';
import { ExclamationCircleFilled } from '@ant-design/icons';
import type { Session } from '../../entity/session';
import type { VoidFunction } from '../../utils/interface';
import { useEventBus } from '../../utils/eventBus';
import { queryTopSummary } from '../../utils/RequestUtils';
import { addResizeEvent, chartVisbilityListener, COLOR, commonEchartsOptions, notZero } from '../Common';
import Filter from './Filter';
import type { ConditionDataType } from './Filter';
import StatisticsTable from './StatisticsTable';
import BaseInfo from './BaseInfo';
import { CommunicatorContainer } from '../communicatorContainer/CommunicatorContainer';
import PpBandwidthAnalysis from './PpBandwidthAnalysis';
import i18n from 'ascend-i18n';
import { HelpIcon } from 'ascend-icon';
import { Layout } from 'ascend-layout';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Tooltip } from 'ascend-components';
import { chartColors } from 'ascend-utils';

interface SummaryDataType {
    [propName: string]: any;
    rankId: string ;
    prepareTime: number;
    computingTime: number;
    communicationNotOverLappedTime: number;
    communicationOverLappedTime: number;
    freeTime: number;
    computeTimeRatio?: string | number;
    communicationTimeRatio?: string | number;
    pureComputingTime?: string | number;
}

export interface AdviceInfo {
    communication: number;
    compute: number;
    free: number;
};

interface AdviceAndSummary {
    advice: AdviceInfo;
    summaryList: SummaryDataType[];
}

const baseOptionLegendData = [
    { name: 'Preparing', textStyle: { color: COLOR.GREY_50 } },
    { name: 'Pure Computing', textStyle: { color: COLOR.GREY_50 } },
    { name: 'Communication(Overlapped)', textStyle: { color: COLOR.GREY_50 } },
    { name: 'Communication(Not Overlapped)', textStyle: { color: COLOR.GREY_50 } },
    { name: 'Free', textStyle: { color: COLOR.GREY_50 } },
    { name: 'Total Computing Ratio', textStyle: { color: COLOR.GREY_50 } },
    { name: 'Communication Ratio', textStyle: { color: COLOR.GREY_50 } },
];

const commonSeries: any = {
    type: 'bar',
    stack: 'Ad',
    emphasis: {
        focus: 'series',
    },
    tooltip: {
        valueFormatter: function (value: any) {
            return `${value} μs`;
        },
    },
    data: [],
};

const baseSeries = [
    { id: 'preparing', name: 'Preparing', ...commonSeries },
    {
        id: 'computingTime',
        name: 'Total Computing',
        type: 'custom',
        renderItem: (): string => {
            return '';
        },
        tooltip: {
            valueFormatter: (value: string | number): string => {
                return `${value} μs`;
            },
        },
    },
    {
        id: 'purecompute',
        name: 'Pure Computing',
        ...commonSeries,
    },
    {
        id: 'communicationOverLappedTime',
        name: 'Communication(Overlapped)',
        ...commonSeries,
    },
    {
        id: 'communicationNotOverLappedTime',
        name: 'Communication(Not Overlapped)',
        ...commonSeries,
    },
    {
        id: 'freeTime',
        name: 'Free',
        ...commonSeries,
    },
    {
        name: 'Total Computing Ratio',
        type: 'line',
        yAxisIndex: 1,
        tooltip: {
            valueFormatter: function (value: any): string {
                return `${value} %`;
            },
        },
        data: [],
    },
    {
        name: 'Communication Ratio',
        type: 'line',
        yAxisIndex: 1,
        tooltip: {
            valueFormatter: function (value: any): string {
                return `${value} %`;
            },
        },
        data: [],
    },
];

const baseOption: any = {
    color: chartColors,
    tooltip: commonEchartsOptions.tooltip,
    legend: {
        data: [],
        tooltip: {
            show: true,
            formatter: function () {
                const div = document.createElement('div');
                div.className = 'legend-tooltip';
                div.append(i18n.t('chart:switchTooltip'));
                return div;
            },
        },
    },
    grid: {
        left: '3%',
        right: '4%',
        bottom: '3%',
        containLabel: true,
    },
    xAxis: [
        {
            type: 'category',
            data: [],
            name: 'Rank',
            nameLocation: 'start',
            nameGap: 15,
            nameTextStyle: { fontWeight: 'bold', fontSize: '1rem', padding: [0, 10, 0, 0] },
            axisLabel: {
                color: COLOR.GREY_40,
            },
        },
    ],
    yAxis: [
        {
            type: 'value',
            name: 'Time(μs)',
            axisLabel: {
                formatter: '{value}',
                color: COLOR.GREY_40,
            },
        },
        {
            type: 'value',
            name: 'Ratio',
            min: 0,
            axisLabel: {
                formatter: '{value}%',
                color: COLOR.GREY_40,
            },
            splitLine: commonEchartsOptions.splitLineY,
        },
    ],
    series: [],
};

const getLegendData = (): any[] => {
    return baseOptionLegendData.map(legendItem => ({ ...legendItem, name: i18n.t(legendItem.name, { ns: 'summary' }) }));
};
const getBaseOptionSeries = (): any[] => {
    return baseSeries.map(serieItem => ({
        ...serieItem,
        name: i18n.t(serieItem.name, { ns: 'summary' }),
    }));
};

function checkIfContainsFieldPreparing(data: SummaryDataType[]): boolean {
    if (data.length === 0 || data[0].prepareTime < 0) { // 后台返回的预处理时间小于零，说明是旧版本数据，移除预处理时间相关的字段，兼容一个版本，后续可删除
        return false;
    }
    for (const item of data) {
        if (item.prepareTime > 0) {
            return true;
        }
    }
    // 后台返回的数据全部为0，说明是旧版本数据
    return false;
}

function wrapData(data: SummaryDataType[]): any {
    const list = ['prepareTime', 'computingTime', 'communicationNotOverLappedTime', 'communicationOverLappedTime', 'freeTime'];
    const totalFields = ['prepareTime', 'computingTime', 'communicationNotOverLappedTime', 'freeTime'];
    let isContainsFieldPreparing = true;
    if (!checkIfContainsFieldPreparing(data)) { // 后台返回的预处理时间小于零，说明是旧版本数据，移除预处理时间相关的字段，兼容一个版本，后续可删除
        isContainsFieldPreparing = false;
        list.shift();
        totalFields.shift();
    }
    data.forEach(item => {
        if (!isContainsFieldPreparing) {
            item.prepareTime = 0;
        }
        list.forEach(field => {
            item[field] = isNaN(item[field]) ? 0 : Number(item[field].toFixed(4));
        });
        let total = 0;
        totalFields.forEach(field => {
            total += item[field];
        });
        item.pureComputingTime = Number((item.computingTime - item.communicationOverLappedTime)
            .toFixed(2));
        item.computeTimeRatio = Number((100 * item.computingTime / notZero(total)).toFixed(2));
        item.communicationTimeRatio = Number((100 * item.communicationNotOverLappedTime / notZero(total)).toFixed(2));
    });
    baseOption.xAxis[0].data = data.map(item => item.rankId);
    // computingTime为总计算时间
    const order: Array<keyof SummaryDataType> = ['prepareTime', 'computingTime', 'pureComputingTime', 'communicationOverLappedTime',
        'communicationNotOverLappedTime', 'freeTime', 'computeTimeRatio', 'communicationTimeRatio'];
    const legendDataTemp = getLegendData();
    const seriesTemp = [...getBaseOptionSeries()];
    if (!isContainsFieldPreparing) { // 如果不包含预处理字段，移除相关的图标和tooltips信息
        order.shift();
        legendDataTemp.shift();
        seriesTemp.shift();
    }
    baseOption.legend.data = legendDataTemp;
    baseOption.series = seriesTemp;
    for (let i = 0; i < order.length; i++) {
        baseOption.series[i].data = data.map(item => item[order[i]]);
    }
    return baseOption;
}

async function initCharts(data: any, handleClick: VoidFunction): Promise<void> {
    const chartDom = document.getElementById('overview-chart');
    if (chartDom === null || chartDom.offsetParent === null) {
        return;
    }
    echarts.init(chartDom).dispose();
    const myChart = echarts.init(chartDom);
    myChart.setOption(wrapData(data));
    myChart.on('click', handleClick);
    addResizeEvent(myChart);
}
export const useHit = (containsPreparing: boolean): React.ReactElement => {
    const { t } = useTranslation('summary');
    const hit = t(containsPreparing ? 'Computation/CommunicationDescribeWithPreparing' : 'Computation/CommunicationDescribe',
        { returnObjects: true }) as string[];
    return (<Tooltip
        overlayClassName={'width-auto'}
        title={
            (
                <div style={{ padding: '1rem' }}>
                    {hit?.map((item: string, index: number) => <div key={index}>{item}</div>)}
                    <div style={{ marginTop: '2rem' }}>
                        <ExclamationCircleFilled style={{ marginRight: '10px' }}/>
                        {t('Computation/CommunicationLastDescribe')}</div>
                </div>
            )
        }>
        <HelpIcon style={{ cursor: 'pointer', marginLeft: '3px' }} height={20} width={20}/>
    </Tooltip>);
};

async function GetTopSummary(conditions: ConditionDataType): Promise<AdviceAndSummary> {
    const res: any = await queryTopSummary({
        ...conditions,
        orderBy: conditions.orderBy === 'pureComputingTime' ? 'computingTime' : conditions.orderBy,
    });
    const { advice = {}, summaryList = [] } = res ?? {};
    if (conditions.orderBy === 'pureComputingTime') {
        summaryList.sort((a: any, b: any) =>
            b.computingTime - b.communicationOverLappedTime - a.computingTime + a.communicationOverLappedTime);
    }
    return { advice, summaryList };
};

const ComputationCommunicationOverview = observer(({ session }: { session: Session }): JSX.Element => {
    const [dataSource, setDatasource] = useState<SummaryDataType[]>([]);
    const [allDataSource, setAllDatasource] = useState<SummaryDataType[]>([]);
    const [selected, setSelected] = useState({ rankId: '', step: '' });
    const [chartRankId, setChartRankId] = useState('');
    const [slowAdvice, setSlowAdvice] = useState<AdviceInfo>({ communication: 0, compute: 0, free: 0 });
    const containsPreparing = useMemo(() => checkIfContainsFieldPreparing(dataSource), [dataSource]);
    const { t } = useTranslation('summary');

    chartVisbilityListener('overview-chart', () => {
        initCharts(dataSource, handleClick);
    });
    useEffect(() => {
        setTimeout(() => {
            initCharts(dataSource, handleClick);
        });
    }, [dataSource, t]);
    useEffect(() => {
        setSelected({ ...selected, rankId: chartRankId });
    }, [chartRankId]);
    const handleFilterChange = async (conditions: ConditionDataType, doQuery?: boolean): Promise<void> => {
        if (!session.clusterCompleted) {
            setAllDatasource([]);
            setDatasource([]);
            setSelected({ rankId: '', step: '' });
            return;
        }
        if (doQuery === false) {
            let data = [...allDataSource]; data = data.slice(0, conditions.top);
            setDatasource(data);
            setSelected({ ...selected, step: conditions.step, rankId: data[0]?.rankId });
            setChartRankId(data[0]?.rankId);
            return;
        }
        const { advice, summaryList } = await GetTopSummary(conditions);
        const data = [...summaryList];
        setAllDatasource(data);
        setSlowAdvice(advice);
        setDatasource(conditions.top === 0 ? summaryList : summaryList.slice(0, conditions.top));
        setSelected({ ...selected, step: conditions.step, rankId: data[0]?.rankId });
        setChartRankId(data[0]?.rankId);
    };

    const handleClick = (param: any): void => {
        const { name: rankId } = param;
        setChartRankId(rankId);
    };
    return session.renderId > 0
        ? (<OverviewCom handleFilterChange={handleFilterChange} session={session}
            dataSource={dataSource} advice={slowAdvice} selected={selected} containsPreparing={containsPreparing}/>)
        : <></>;
});
function OverviewCom({ handleFilterChange, dataSource, selected, advice, session, containsPreparing }: any): JSX.Element {
    const [pipelineVisible, setPipelineVisible] = useState(false);
    const { t } = useTranslation('summary');
    useEventBus('setActiveTab', (data) => {
        setPipelineVisible(data === 'pp');
    });
    return <Layout>
        <BaseInfo session={session}/>

        <CollapsiblePanel title={t('Parallel Strategy Analysis')}>
            <CommunicatorContainer session={session}></CommunicatorContainer>
            <div className={pipelineVisible ? 'hide' : ''}>
                <CollapsiblePanel
                    secondary
                    title={<div className={'flex items-center'}>{t('Computation/CommunicationOverview')}{useHit(containsPreparing)}</div>}
                    headerStyle={{ padding: 0 }}
                    contentStyle={{ paddingLeft: 0, paddingRight: 0 }}>
                    <Filter handleFilterChange={handleFilterChange} session={session} visible={!pipelineVisible}/>
                    <div id={'overview-chart'} style={{ height: '400px' }} ></div>
                    <StatisticsTable {...selected} advice={advice} session={session}/>
                </CollapsiblePanel>
            </div>
            <div className={pipelineVisible ? '' : 'hide'}>
                <PpBandwidthAnalysis session={session}></PpBandwidthAnalysis>
            </div>
        </CollapsiblePanel>
    </Layout>;
};

export default ComputationCommunicationOverview;
