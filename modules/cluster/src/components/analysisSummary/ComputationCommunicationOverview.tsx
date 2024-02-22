/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import * as echarts from 'echarts';
import { Tooltip } from 'antd';
import { observer } from 'mobx-react';
import { QuestionCircleFilled, ExclamationCircleFilled } from '@ant-design/icons';
import { Session } from '../../entity/session';
import { VoidFunction } from '../../utils/interface';
import { useEventBus } from '../../utils/eventBus';
import { queryTopSummary } from '../../utils/RequestUtils';
import { addResizeEvent, COLOR } from '../Common';
import Filter, { ConditionDataType } from './Filter';
import StatisticsTable from './StatisticsTable';
import SummaryTable from './SummaryTable';
import BaseInfo from './BaseInfo';
import { CommunicatorContainer } from '../communicatorContainer/CommunicatorContainer';
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
            data: [],
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
            data: [],
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
            data: [],
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
            data: [],
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
            data: [],
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
            data: [],
        },
    ],
};
function wrapData(data: SummaryDataType[]): any {
    data.forEach(item => {
        const list = ['computingTime', 'communicationNotOverLappedTime', 'communicationOverLappedTime', 'freeTime'];
        list.forEach(field => {
            item[field] = Number(item[field].toFixed(4));
        });
        const totalFields = ['computingTime', 'communicationNotOverLappedTime', 'freeTime'];
        let total = 0;
        totalFields.forEach(field => {
            total += item[field];
        });
        item.computingTime = item.computingTime - item.communicationOverLappedTime;
        item.computeTimeRatio = Number((100 * item.computingTime / total).toFixed(2));
        item.communicationTimeRatio = Number((100 * item.communicationNotOverLappedTime / total).toFixed(2));
    });
    baseOption.xAxis[0].data = data.map(item => item.rankId);
    const order: Array<keyof SummaryDataType> = ['computingTime', 'communicationNotOverLappedTime',
        'communicationOverLappedTime', 'freeTime', 'computeTimeRatio', 'communicationTimeRatio'];
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
            <div>Computing = Sum of kernel time on NPU - Communication(Overlapped)</div>
            <div>Total Time = Computing + Communication(Overlapped) + Communication(Not Overlapped) + Free</div>
            <div>Computing Ratio = Computing / Total Time</div>
            <div>Communication Ratio = Communication(Not Overlapped) / Total Time</div>
            <div style={{ marginTop: '2rem' }}><ExclamationCircleFilled style={{ marginRight: '10px' }}/>
                Click Bar Chart Display The Single Rank Computation / Communication Detail</div>
        </div>
    )
}>
    <QuestionCircleFilled style={{ cursor: 'pointer', margin: '0 10px' }}/>
</Tooltip>);

const ComputationCommunicationOverview = observer(({ session, active = true }: { session: Session ;active?: boolean}): JSX.Element => {
    const [dataSource, setDatasource] = useState<SummaryDataType[]>([]);
    const [allDataSource, setAllDatasource] = useState<SummaryDataType[]>([]);
    const [selected, setSelected] = useState({ rankId: '', step: '' });
    useEffect(() => {
        setTimeout(() => {
            if (active) {
                initCharts(dataSource, handleClick);
            }
        });
    }, [dataSource, active]);
    const handleFilterChange = async (conditions: ConditionDataType, doQuery?: boolean): Promise<void> => {
        if (doQuery === false) {
            let data = [...allDataSource];
            data = data.slice(0, conditions.top);
            setDatasource(data);
            setSelected({ ...selected, step: conditions.step, rankId: data[0]?.rankId });
            return;
        }
        const res: any = await queryTopSummary(conditions);
        const { summaryList } = res;
        const data = [...summaryList];
        setAllDatasource(data);
        setDatasource(conditions.top === 0 ? summaryList : summaryList.slice(0, conditions.top));
        setSelected({ ...selected, step: conditions.step, rankId: data[0]?.rankId });
    };

    const handleClick = (param: any): void => {
        const { name: rankId } = param;
        setSelected({ ...selected, rankId });
    };
    return session.renderId > 0
        ? (<OverviewCom handleFilterChange={handleFilterChange} session={session}
            dataSource={dataSource} selected={selected}/>)
        : <></>;
});
const OverviewCom = ({ handleFilterChange, dataSource, selected, session }: any): JSX.Element => {
    const [pipelineVisible, setPipelineVisible] = useState(true);
    useEventBus('setActiveTab', (data) => {
        setPipelineVisible(data === 'pp');
    });
    return <div className={'text-selectable'}
        style={{ textAlign: 'left', padding: '0 20px', overflow: 'auto', height: '100%', width: '100%' }}>
        <BaseInfo session={session}/>
        <CommunicatorContainer session={session}></CommunicatorContainer>
        <div className={pipelineVisible ? 'hide' : ''}>
            <div>
                <div className={'common-title-bottom'}>Computation/Communication Overview{hit}</div>
                <Filter handleFilterChange={handleFilterChange} session={session} visible={!pipelineVisible}/>
                <div id={'overview-chart'} style={{ height: '400px' }} ></div>
            </div>
            <div style={{ padding: '0 3rem' }}>
                <SummaryTable dataSource={dataSource} style={{ display: 'none' }}/>
                <StatisticsTable {...selected} session={session}/>
            </div>
        </div>
        <div className={pipelineVisible ? '' : 'hide'}>
            <PpBandwidthAnalysis session={session}></PpBandwidthAnalysis>
        </div>
    </div>;
};

export default ComputationCommunicationOverview;
