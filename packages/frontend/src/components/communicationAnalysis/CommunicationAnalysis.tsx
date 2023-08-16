/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useState } from 'react';
import { ArrowLeftOutlined } from '@ant-design/icons';
import { Breadcrumb } from 'antd';
import { Session } from '../../entity/session';
import Help from './Help';
import Filter, { conditionDataType } from './Filter';
import CommunicationTimeTable, { DataType as tableDataType } from './CommunicationTimeTable';
import CommunicationTimeChart, { dataType as chartDataType }
    from './CommunicationTimeChart';
import CommunicationMatrix from './CommunicationMatrix';
import BandwidthAnalysis from './BandwidthAnalysis';
import { Space, Tan } from '../Common';
import { queryCommunication } from '../../utils/RequestUtils';

const Operators = ({ returnHome, rankId, operatorName, iterationId, session }: any): JSX.Element => {
    return (
        <div className={'fullbox'} style={{ padding: '0 20px', overflow: 'auto' }}>
            <Breadcrumb>
                <Breadcrumb.Item onClick={returnHome }>
                    <a><ArrowLeftOutlined /><Space length={10}/><span>Communication Duration Analysis</span></a>
                </Breadcrumb.Item>
                <Breadcrumb.Item>{operatorName}(RankId {rankId})</Breadcrumb.Item>
            </Breadcrumb>
            <BandwidthAnalysis iterationId={iterationId} rankId={rankId} operatorName={operatorName}/>
        </div>
    );
};

interface showDataType{
    chartData: chartDataType;
    tableData: [];
}

const searchData = async (conditions: conditionDataType): Promise<showDataType> => {
    const res = await queryCommunication(conditions);
    res.duration.forEach((item: any) => { item.rankId = item.rank_id; });
    return { chartData: wrapChartData(res.duration), tableData: res.duration };
};
const wrapChartData = (data: tableDataType[]): chartDataType => {
    // 显示字段
    const fields = [ 'rank_id', 'elapse_time', 'transit_time', 'synchronization_time',
        'wait_time', 'synchronization_time_ratio', 'wait_time_ratio' ];
    const chartData: chartDataType = {} as chartDataType;
    fields.forEach(field => {
        chartData[field] = data.map((item: any) => item[field]);
    });
    return chartData;
};
const isShow = (name: string): boolean => {
    return name === 'CommunicationDurationAnalysis';
};

const CommunicationAnalysis = observer(function ({ session }: { session: Session }) {
    const [ rankId, setRankId ] = useState('');
    const [ showData, setShowData ] = useState<showDataType>({
        chartData: {} as chartDataType,
        tableData: [],
    });
    const [ conditions, setConditions ] = useState<conditionDataType>(
        { iterationId: '', rankIds: [], operatorName: '', type: '' });
    const showOperator = (rankId: string): void => {
        setRankId(rankId);
    };
    const returnHome = (): void => { setRankId(''); };
    const handleFilterChange = async(conditions: conditionDataType): Promise<void> => {
        setConditions(conditions);
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
                main={ <div style={{ padding: '0 16px' }}>
                    <CommunicationTimeChart dataSource={showData.chartData}/>
                    <CommunicationTimeTable showOperator={showOperator} dataSource={showData.tableData}
                        conditions={conditions} updateSort={(data) => {
                            setShowData({ ...showData, chartData: wrapChartData(data) });
                        }}/>
                </div>
                }
                dragSize={400}
                id={'communication-analysis'}
                style={{ display: isShow('CommunicationDurationAnalysis') ? 'block' : 'none' }}
            />
            {/* 通信矩阵 */}
            <CommunicationMatrix isShow={isShow('CommunicationMatrix')}/>
            {/* 带宽分析 */}
            { rankId !== '' && <Operators iterationId={conditions.iterationId}
                rankId={rankId} session={session} returnHome={returnHome} operatorName={conditions.operatorName} /> }
        </div>
    );
});

export default CommunicationAnalysis;
