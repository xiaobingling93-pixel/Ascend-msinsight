/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import * as echarts from 'echarts';
import React, { useEffect, useState } from 'react';
import Filter, { ConditionDataType } from './Filter';
import StatisticsTable from './StatisticsTable';
import { VoidFunction } from '../../utils/interface';
import SummaryTable from './SummaryTable';
import { queryTopSummary } from '../../utils/RequestUtils';
import BaseInfo, { BaseInfoDataType, defaultBaseInfo } from './BaseInfo';
import { addResizeEvent, COLOR, formatDate } from '../Common';
import { Tooltip } from 'antd';
import { QuestionCircleFilled, ExclamationCircleFilled } from '@ant-design/icons';
import { Session } from '../../entity/session';
import { communicator, CommunicatorContainer } from '../communicatorContainer/CommunicatorContainer';
import { useEventBus } from '../../utils/eventBus';
import PpBandwidthAnalysis from './PpBandwidthAnalysis';

interface SummaryDataType{
    rankId: string ;
    computingTime: number;
    communicationNotOverLappedTime: number;
    communicationOverLappedTime: number;
    freeTime: number;
    ComputeTimeRatio?: string | number;
    CommunicationTimeRatio?: string | number;
    [propName: string]: any;
}

const baseOption: any = {
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
    legend: {
        data: [
            { name: 'Computing', textStyle: { color: COLOR.Grey50 } },
            { name: 'Communication(Not Overlapped)', textStyle: { color: COLOR.Grey50 } },
            { name: 'Communication(Overlapped)', textStyle: { color: COLOR.Grey50 } },
            { name: 'Free', textStyle: { color: COLOR.Grey50 } },
            { name: 'Computing Ratio', textStyle: { color: COLOR.Grey50 } },
            { name: 'Communication Ratio', textStyle: { color: COLOR.Grey50 } },
        ],
        tooltip: {
            show: true,
            formatter: function () {
                const div = document.createElement('div');
                div.className = 'legend-tooltip';
                div.append('Click to Switch Display and Hide');
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
            data: [ ],
            name: 'Rank',
            nameLocation: 'start',
            nameGap: 15,
            nameTextStyle: { fontWeight: 'bold', fontSize: '1rem', padding: [ 0, 10, 0, 0 ] },
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
            splitLine: {
                lineStyle: {
                    color: COLOR.Grey20,
                    type: 'dashed',
                },
            },
        },
    ],
    series: [
        {
            id: 'compute',
            name: 'Computing',
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
            data: [ ],
        },
        {
            id: 'communicationNotOverLappedTime',
            name: 'Communication(Not Overlapped)',
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
            data: [ ],
        },
        {
            id: 'communicationOverLappedTime',
            name: 'Communication(Overlapped)',
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
            data: [ ],
        },
        {
            id: 'freeTime',
            name: 'Free',
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
            data: [ ],
        },
        {
            name: 'Computing Ratio',
            type: 'line',
            yAxisIndex: 1,
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + ' %';
                },
            },
            data: [ ],
        },
        {
            name: 'Communication Ratio',
            type: 'line',
            yAxisIndex: 1,
            tooltip: {
                valueFormatter: function (value: any) {
                    return value + ' %';
                },
            },
            data: [ ],
        },
    ],
};
function wrapData(data: SummaryDataType[]): any {
    data.forEach(item => {
        const list = [ 'computingTime', 'communicationNotOverLappedTime', 'communicationOverLappedTime', 'freeTime' ];
        list.forEach(field => {
            item[field] = Number(item[field].toFixed(4));
        });
        const totalFields = [ 'computingTime', 'communicationNotOverLappedTime', 'freeTime' ];
        let total = 0;
        totalFields.forEach(field => {
            total += item[field];
        });
        item.computeTimeRatio = Number((100 * item.computingTime / total).toFixed(2));
        item.communicationTimeRatio = Number((100 * item.communicationNotOverLappedTime / total).toFixed(2));
    });
    baseOption.xAxis[0].data = data.map(item => item.rankId);
    const order: Array<keyof SummaryDataType> = [ 'computingTime', 'communicationNotOverLappedTime',
        'communicationOverLappedTime', 'freeTime', 'computeTimeRatio', 'communicationTimeRatio' ];
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
export const hit = (<Tooltip title={
    (
        <div style={{ background: 'var(--grey100)', padding: '1rem' }}>
            <div>Total Time = Computing + Communication(Not Overlapped) + Free</div>
            <div>Computing Ratio = Computing / Total Time</div>
            <div>Communication Ratio = Communication(Not Overlapped) / Total Time</div>
            <div style={{ marginTop: '2rem' }}><ExclamationCircleFilled style={{ marginRight: '10px' }}/>
                Click Bar Chart Display The Single Rank Computation / Communication Detail</div>
        </div>
    )
}>
    <QuestionCircleFilled style={{ cursor: 'pointer', margin: '0 10px' }}/>
</Tooltip>);

const ComputationCommunicationOverview = ({ session, active }: { session: Session ;active: boolean}): JSX.Element => {
    const [ groupData, setGroupData ] = useState({ rankList: [], stepList: [], init: false });
    const [ dataSource, setDatasource ] = useState<SummaryDataType[]>([]);
    const [ allDataSource, setAllDatasource ] = useState<SummaryDataType[]>([]);
    const [ selected, setSelected ] = useState({ rankId: '', timeFlag: '' });
    const [ baseInfo, setBaseInfo ] = useState<BaseInfoDataType>(defaultBaseInfo);
    useEffect(() => {
        handleFilterChange({ step: 'All', rankIds: [], orderBy: 'computingTime', top: 0 });
    }, [ ]);
    useEffect(() => {
        setTimeout(() => {
            initCharts(dataSource, handleClick);
        },
        );
    }, [ dataSource, active ]);
    const handleFilterChange = async (conditions: ConditionDataType, doQuery?: boolean): Promise<void> => {
        if (doQuery === false) {
            let data = [...allDataSource];
            data = data.slice(0, conditions.top);
            setDatasource(data);
            return;
        }
        const res: any = await queryTopSummary(conditions);
        const { summaryList, rankList = [], stepList = [] } = res.result;
        const data = [...summaryList];
        setAllDatasource(data);
        setBaseInfo({ ...res.result, collectStartTime: formatDate(new Date(res.result.collectStartTime)) });
        if (!groupData.init) {
            setGroupData({ rankList, stepList, init: true });
            setSelected({ ...selected, rankId: rankList[0] });
        }
        setDatasource(conditions.top === 0 ? summaryList : summaryList.slice(0, conditions.top));
    };

    const handleClick = (param: any): void => {
        const { name: rankId, seriesId: timeFlag } = param;
        setSelected({ rankId, timeFlag });
    };
    return <OverviewCom baseInfo={baseInfo} handleFilterChange={handleFilterChange} session={session}
        groupData={groupData} dataSource={dataSource} selected={selected}/>;
};
const OverviewCom = ({ baseInfo, handleFilterChange, groupData, dataSource, selected, session }: any): JSX.Element => {
    const [ pipelineVisible, setPipelineVisible ] = useState(true);
    useEventBus('activeCommunicator', (data) => {
        if (data === undefined) {
            setPipelineVisible(true);
        } else {
            const selectCommunicator = data as communicator;
            setPipelineVisible(selectCommunicator.name.startsWith('stage'));
        }
    });
    return <div className={'text-selectable'}
        style={{ textAlign: 'left', padding: '0 20px', overflow: 'auto', height: '100%' }}>
        <BaseInfo data={baseInfo} session={session}/>
        <CommunicatorContainer session={session}></CommunicatorContainer>
        <div className={pipelineVisible ? 'hide' : ''}>
            <div>
                <div className={'common-title-bottom'}>Computation/Communication Overview{hit}</div>
                <Filter handleFilterChange={handleFilterChange} groupData={groupData} session={session}/>
                <div id={'overview-chart'} style={{ height: '400px' }} ></div>
            </div>
            <div style={{ padding: '0 3rem' }}>
                <SummaryTable dataSource={dataSource} style={{ display: 'none' }}/>
                <StatisticsTable {...selected}/>
            </div>
        </div>
        <div className={pipelineVisible ? '' : 'hide'}>
            <PpBandwidthAnalysis session={session}></PpBandwidthAnalysis>
        </div>
    </div>;
};

export default ComputationCommunicationOverview;
