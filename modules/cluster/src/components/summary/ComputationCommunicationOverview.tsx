/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import * as echarts from 'echarts';
import { observer } from 'mobx-react';
import { QuestionCircleFilled, ExclamationCircleFilled } from '@ant-design/icons';
import { Session } from '../../entity/session';
import { VoidFunction } from '../../utils/interface';
import { useEventBus } from '../../utils/eventBus';
import { queryTopSummary } from '../../utils/RequestUtils';
import { addResizeEvent, chartVisbilityListener, COLOR, commonEchartsOptions, notZero, StyledTooltip } from '../Common';
import Filter, { ConditionDataType } from './Filter';
import StatisticsTable from './StatisticsTable';
import SummaryTable from './SummaryTable';
import BaseInfo from './BaseInfo';
import { CommunicatorContainer } from '../communicatorContainer/CommunicatorContainer';
import PpBandwidthAnalysis from './PpBandwidthAnalysis';
import i18n from '../../i18n';

interface SummaryDataType{
    [propName: string]: any;
    rankId: string ;
    prepareTime: number;
    computingTime: number;
    communicationNotOverLappedTime: number;
    communicationOverLappedTime: number;
    freeTime: number;
    ComputeTimeRatio?: string | number;
    CommunicationTimeRatio?: string | number;
    computingTimeTransfer?: string | number;
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
    { name: 'Preparing', textStyle: { color: COLOR.Grey50 } },
    { name: 'Pure Computing', textStyle: { color: COLOR.Grey50 } },
    { name: 'Communication(Not Overlapped)', textStyle: { color: COLOR.Grey50 } },
    { name: 'Communication(Overlapped)', textStyle: { color: COLOR.Grey50 } },
    { name: 'Free', textStyle: { color: COLOR.Grey50 } },
    { name: 'Computing Ratio', textStyle: { color: COLOR.Grey50 } },
    { name: 'Communication Ratio', textStyle: { color: COLOR.Grey50 } },
];

const commonSeries: any = {
    type: 'bar',
    stack: 'Ad',
    emphasis: {
        focus: 'series',
    },
    tooltip: {
        valueFormatter: function (value: any) {
            return value + ' μs';
        },
    },
    data: [],
};

const getBaseOptionSeries = (): any[] => {
    return [
        { id: 'preparing', name: i18n.t('Preparing', { ns: 'summary' }), ...commonSeries },
        {
            id: 'purecompute',
            name: i18n.t('Pure Computing', { ns: 'summary' }),
            ...commonSeries,
        },
        {
            id: 'communicationNotOverLappedTime',
            name: i18n.t('Communication(Not Overlapped)', { ns: 'summary' }),
            ...commonSeries,
        },
        {
            id: 'communicationOverLappedTime',
            name: i18n.t('Communication(Overlapped)', { ns: 'summary' }),
            ...commonSeries,
        },
        {
            id: 'freeTime',
            name: i18n.t('Free', { ns: 'summary' }),
            ...commonSeries,
        },
        {
            name: i18n.t('Computing Ratio', { ns: 'summary' }),
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
            name: i18n.t('Communication Ratio', { ns: 'summary' }),
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
};

const baseOption: any = {
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
                color: COLOR.Grey40,
            },
        },
    ],
    yAxis: [
        {
            type: 'value',
            name: 'Time(μs)',
            axisLabel: {
                formatter: '{value}',
                color: COLOR.Grey40,
            },
        },
        {
            type: 'value',
            name: 'Ratio',
            min: 0,
            axisLabel: {
                formatter: '{value}%',
                color: COLOR.Grey40,
            },
            splitLine: commonEchartsOptions.splitLineY,
        },
    ],
    series: [],
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
        item.computingTimeTransfer = Number((item.computingTime - item.communicationOverLappedTime)
            .toFixed(2));
        item.computeTimeRatio = Number((100 * item.computingTime / notZero(total)).toFixed(2));
        item.communicationTimeRatio = Number((100 * item.communicationNotOverLappedTime / notZero(total)).toFixed(2));
    });
    baseOption.xAxis[0].data = data.map(item => item.rankId);
    const order: Array<keyof SummaryDataType> = ['prepareTime', 'computingTimeTransfer', 'communicationNotOverLappedTime',
        'communicationOverLappedTime', 'freeTime', 'computeTimeRatio', 'communicationTimeRatio'];
    const legendDataTemp = [...baseOptionLegendData];
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
export const useHit = (): React.ReactElement => {
    const { t } = useTranslation('summary');
    const hit = t('Computation/CommunicationDescribe', { returnObjects: true }) as string[];
    return (<StyledTooltip
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
        <QuestionCircleFilled style={{ cursor: 'pointer', margin: '0 10px' }}/>
    </StyledTooltip>);
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
            dataSource={dataSource} advice={slowAdvice} selected={selected}/>)
        : <></>;
});
const OverviewCom = ({ handleFilterChange, dataSource, selected, advice, session }: any): JSX.Element => {
    const [pipelineVisible, setPipelineVisible] = useState(true);
    const { t } = useTranslation('summary');
    useEventBus('setActiveTab', (data) => {
        setPipelineVisible(data === 'pp');
    });
    return <div className={'text-selectable'}
        style={{ textAlign: 'left', padding: '0 20px', overflow: 'auto', height: '100%', width: '100%' }}>
        <BaseInfo session={session}/>
        <CommunicatorContainer session={session}></CommunicatorContainer>
        <div className={pipelineVisible ? 'hide' : ''}>
            <div>
                <div className={'common-title-bottom'}>{t('Computation/CommunicationOverview')}{useHit()}</div>
                <Filter handleFilterChange={handleFilterChange} session={session} visible={!pipelineVisible}/>
                <div id={'overview-chart'} style={{ height: '400px' }} ></div>
            </div>
            <div style={{ padding: '0 3rem' }}>
                <SummaryTable dataSource={dataSource} style={{ display: 'none' }}/>
                <StatisticsTable {...selected} advice={advice} session={session}/>
            </div>
        </div>
        <div className={pipelineVisible ? '' : 'hide'}>
            <PpBandwidthAnalysis session={session}></PpBandwidthAnalysis>
        </div>
    </div>;
};

export default ComputationCommunicationOverview;
