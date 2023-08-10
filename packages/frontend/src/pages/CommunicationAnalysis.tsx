/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useState } from 'react';
import { ArrowLeftOutlined } from '@ant-design/icons';
import { Breadcrumb } from 'antd';
import { Session } from '../entity/session';
import Help from '../components/communicationAnalysis/Help';
import Filter, { conditionDataType } from '../components/communicationAnalysis/Filter';
import CommunicationTimeTable from '../components/communicationAnalysis/CommunicationTimeTable';
import CommunicationTimeChart, { dataType as chartDataType }
    from '../components/communicationAnalysis/CommunicationTimeChart';
import CommunicationMatrix from '../components/communicationAnalysis/CommunicationMatrix';
import BandwidthAnalysis from '../components/communicationAnalysis/BandwidthAnalysis';
import { Space, Tan } from '../components/communicationAnalysis/Common';

const defaultValue: any = [
    {
        'Rank ID': '0',
        'Elapse Time(ms)': 0.3139404296875,
        'Transit Time(ms)': 0.022670000000000003,
        'Wait Time(ms)': 0.08212999999999997,
        'Synchronization Time(ms)': 0.02225,
        'Wait Time Ratio': 0.7837,
        'Synchronization Time Ratio': 0.4953,
        'Idle Time(ms)': 3,
        'Communication Bandwidth Info': {
            RDMA: {
                'Transit Size(MB)': 0,
                'Transit Time(ms)': 0,
                'Bandwidth(GB/s)': 0,
                'Bandwidth(Utilization)': 0.0,
                'Large Packet Ratio': 0,
                'Size Distribution': {},
            },
            HCCS: {
                'Transit Size(MB)': 0.0146484375,
                'Transit Time(ms)': 0.045090000000000005,
                'Bandwidth(GB/s)': 0.3249,
                'Bandwidth(Utilization)': 0.0181,
                'Large Packet Ratio': 0.0,
                'Size Distribution': {
                    0.0003662109375: 36.0,
                    0.000244140625: 6.0,
                },
            },
            PCIE: {
                'Transit Size(MB)': 0.0048828125,
                'Transit Time(ms)': 0.02176,
                'Bandwidth(GB/s)': 0.2244,
                'Bandwidth(Utilization)': 0.0112,
                'Large Packet Ratio': 0.0,
                'Size Distribution': {
                    0.000244140625: 2.0,
                    0.0003662109375: 12.0,
                },
            },
            SDMA: {
                'Transit Size(MB)': 0.01953125,
                'Transit Time(ms)': 0.06685,
                'Bandwidth(GB/s)': 0.2922,
                'Bandwidth(Utilization)': 0,
                'Large Packet Ratio': 0,
                'Size Distribution': {},
            },
        },
        'Slow Link Suggestion': 'SDMA communication takes most of the time, and is the dominated bottleneck. \nHCCS bandwidth is inefficient, and the bandwidth utilization is 0.02. Because it transported too many small packets, the big packet ratio is only 0.00. \n PCIE bandwidth is inefficient, and the bandwidth utilization is 0.01. Because it transported too many small packets, the big packet ratio is only 0.00. \n ',
    },
    {
        'Rank ID': '1',
        'Elapse Time(ms)': 0.261169921875,
        'Transit Time(ms)': 0.016100000000000003,
        'Wait Time(ms)': 0.012149999999999996,
        'Synchronization Time(ms)': 1e-05,
        'Wait Time Ratio': 0.4301,
        'Synchronization Time Ratio': 0.0006,
        'Communication Bandwidth Info': {
            RDMA: {
                'Transit Size(MB)': 0,
                'Transit Time(ms)': 0,
                'Bandwidth(GB/s)': 0,
                'Bandwidth(Utilization)': 0.0,
                'Large Packet Ratio': 0,
                'Size Distribution': {},
            },
            HCCS: {
                'Transit Size(MB)': 0.0146484375,
                'Transit Time(ms)': 0.04500000000000001,
                'Bandwidth(GB/s)': 0.3255,
                'Bandwidth(Utilization)': 0.0181,
                'Large Packet Ratio': 0.0,
                'Size Distribution': {
                    0.0003662109375: 36.0,
                    0.000244140625: 6.0,
                },
            },
            PCIE: {
                'Transit Size(MB)': 0.0048828125,
                'Transit Time(ms)': 0.02198,
                'Bandwidth(GB/s)': 0.2221,
                'Bandwidth(Utilization)': 0.0111,
                'Large Packet Ratio': 0.0,
                'Size Distribution': {
                    0.0003662109375: 12.0,
                    0.000244140625: 2.0,
                },
            },
            SDMA: {
                'Transit Size(MB)': 0.01953125,
                'Transit Time(ms)': 0.06698000000000001,
                'Bandwidth(GB/s)': 0.2916,
                'Bandwidth(Utilization)': 0,
                'Large Packet Ratio': 0,
                'Size Distribution': {},
            },
        },
        'Slow Link Suggestion': 'SDMA communication takes most of the time, and is the dominated bottleneck. \nHCCS bandwidth is inefficient, and the bandwidth utilization is 0.02. Because it transported too many small packets, the big packet ratio is only 0.00. \n PCIE bandwidth is inefficient, and the bandwidth utilization is 0.01. Because it transported too many small packets, the big packet ratio is only 0.00. \n ',
    },
    { 'Rank ID': '2', 'Elapse Time(ms)': 7, 'Transit Time(ms)': 9, 'Synchronization Time(ms)': 9, 'Wait Time(ms)': 9, 'Synchronization Time Ratio': 3.3, 'Wait Time Ratio': 4.5 },
    { 'Rank ID': '3', 'Elapse Time(ms)': 23.2, 'Transit Time(ms)': 26.4, 'Synchronization Time(ms)': 26.4, 'Wait Time(ms)': 26.4, 'Synchronization Time Ratio': 4.5, 'Wait Time Ratio': 6.3 },
    { 'Rank ID': '4', 'Elapse Time(ms)': 25.6, 'Transit Time(ms)': 28.7, 'Synchronization Time(ms)': 28.7, 'Wait Time(ms)': 28.7, 'Synchronization Time Ratio': 6.3, 'Wait Time Ratio': 18.8 },
    { 'Rank ID': '5', 'Elapse Time(ms)': 76.7, 'Transit Time(ms)': 70.7, 'Synchronization Time(ms)': 70.7, 'Wait Time(ms)': 70.7, 'Synchronization Time Ratio': 10.2, 'Wait Time Ratio': 6 },
    { 'Rank ID': '6', 'Elapse Time(ms)': 135.6, 'Transit Time(ms)': 175.6, 'Synchronization Time(ms)': 175.6, 'Wait Time(ms)': 175.6, 'Synchronization Time Ratio': 20.3, 'Wait Time Ratio': 2.3 },
];

const Operators = ({ returnHome, rankId, session }: any): JSX.Element => {
    return (
        <div className={'fullbox'} style={{ padding: '0 20px', background: 'white', zIndex: 10 }}>
            <Breadcrumb>
                <Breadcrumb.Item onClick={returnHome }>
                    <a><ArrowLeftOutlined /><Space length={10}/><span>Communication Duration Analysis</span></a>
                </Breadcrumb.Item>
                <Breadcrumb.Item>Total HCCL Operators(RankId {rankId})</Breadcrumb.Item>
            </Breadcrumb>
            <BandwidthAnalysis session={session}/>
        </div>
    );
};

interface showDataType{
    chartData: chartDataType;
    tableData: [];
}

const searchData = (conditions: conditionDataType): any => {
    const list = defaultValue;
    // 显示字段
    const fields = [ 'Rank ID', 'Elapse Time(ms)', 'Transit Time(ms)', 'Synchronization Time(ms)',
        'Wait Time(ms)', 'Synchronization Time Ratio', 'Wait Time Ratio' ];
    const chartData: chartDataType = {} as chartDataType;
    fields.forEach(field => {
        chartData[field] = list.map((item: any) => item[field]);
    });

    return { chartData, tableData: list };
};

const CommunicationAnalysis = observer(function ({ session }: { session: Session }) {
    const [ currentWindow, setCurrentWindow ] = useState('CommunicationDurationAnalysis');
    const [ rankId, setRankId ] = useState('');
    const [ showData, setShowData ] = useState<showDataType>({
        chartData: {} as chartDataType,
        tableData: [],
    });
    const isShow = (page: string): boolean => {
        return page === currentWindow;
    };
    const showOperator = (rankId: string): void => {
        setRankId(rankId);
    };
    const returnHome = (): void => { setRankId(''); };
    const handleFilterChange = (conditions: conditionDataType): void => {
        if (currentWindow !== conditions.type) {
            setCurrentWindow(conditions.type);
            return;
        }
        const res = searchData(conditions);
        setShowData(res);
    };
    return (
        <div style={{ textAlign: 'left' }} className={'fullwindow-topflex-bottomstretch'}>
            {/* 筛选条件 */}
            <Filter handleFilterChange={handleFilterChange} session={session} />
            {/* 通信用时分析 */}
            <Tan
                position={'left'}
                drag={<Help />}
                main={
                    <div>
                        <CommunicationTimeChart dataSource={showData.chartData}/>
                        <CommunicationTimeTable showOperator={showOperator} dataSource={showData.tableData}/>
                    </div>
                }
                dragSize={400}
                id={'communication-analysis'}
                style={{ display: isShow('CommunicationDurationAnalysis') ? 'block' : 'none' }}
            />
            {/* 通信矩阵 */}
            <CommunicationMatrix isShow={isShow('CommunicationMatrix')}/>
            {/* 算子详情 */}
            { rankId !== '' && <Operators rankId={rankId} session={session} returnHome={returnHome} /> }
        </div>
    );
});

export default CommunicationAnalysis;
