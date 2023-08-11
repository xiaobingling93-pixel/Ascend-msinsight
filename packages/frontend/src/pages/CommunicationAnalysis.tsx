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
import { communicationAnalysisData } from '../utils/__test__/mockData';

const Operators = ({ returnHome, rankId, session }: any): JSX.Element => {
    return (
        <div className={'fullbox'} style={{ padding: '0 20px' }}>
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

const searchData = async (conditions: conditionDataType): Promise<showDataType> => {
    const list = communicationAnalysisData;
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
    const handleFilterChange = async(conditions: conditionDataType): Promise<void> => {
        if (currentWindow !== conditions.type) {
            setCurrentWindow(conditions.type);
            return;
        }
        const res = await searchData(conditions);
        setShowData(res);
    };
    return (
        <div style={{ textAlign: 'left' }} className={'header-fixed-content-scroll'}>
            {/* 筛选条件 */}
            <Filter handleFilterChange={handleFilterChange} session={session} />
            {/* 通信用时分析 */}
            <Tan
                position={'left'}
                drag={<Help />}
                main={
                    <div style={{ padding: '0 16px' }}>
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
