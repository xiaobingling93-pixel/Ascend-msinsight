/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { ArrowLeftOutlined } from '@ant-design/icons';
import { Breadcrumb } from 'antd';
import { Session } from '../../entity/session';
import Help from './Help';
import Filter, { ConditionDataType } from './Filter';
import CommunicationTimeTable, { DataType, DataType as tableDataType } from './CommunicationTimeTable';
import CommunicationTimeChart, { dataType as chartDataType }
    from './CommunicationTimeChart';
import CommunicationMatrix from './CommunicationMatrix';
import BandwidthAnalysis from './BandwidthAnalysis';
import { notNullObj, Space, StyledTooltip, Tan } from '../Common';
import { queryCommunication, queryCommunicationOperatorLists } from '../../utils/RequestUtils';
import CommunicationTimeAnalysisChart from './CommunicationTimeAnalysisChart';
import type { AnalysisChartData } from './CommunicationTimeAnalysisChart';
import { HelpIcon } from 'lib/Icon';

const Operators = ({ returnHome, rankId, operatorName, iterationId, stage }: any): JSX.Element => {
    const { t } = useTranslation('communication');
    return (
        <div className={'fullbox'} style={{ padding: '0 20px', overflow: 'auto' }}>
            <Breadcrumb>
                <Breadcrumb.Item onClick={returnHome }>
                    <a><ArrowLeftOutlined /><Space length={10}/><span>{t('searchCriteria.CommunicationDurationAnalysis')}</span></a>
                </Breadcrumb.Item>
                <Breadcrumb.Item>{operatorName}(RankId {rankId})</Breadcrumb.Item>
            </Breadcrumb>
            <BandwidthAnalysis iterationId={iterationId} rankId={rankId} operatorName={operatorName} stage={stage}/>
        </div>
    );
};

interface CommunicationAdvice {
    type: string;
    max: number;
    min: number;
    avg: number;
    diff: number;
    time: number;
}

interface showDataType{
    chartData: chartDataType;
    analysisChartData: AnalysisChartData;
    tableData: [];
    adviceData: CommunicationAdvice[];
}

const searchData = async (conditions: ConditionDataType): Promise<showDataType> => {
    if (!notNullObj(conditions)) {
        return { chartData: wrapChartData([]), analysisChartData: { minTime: 0, maxTime: 0, data: [] }, tableData: [], adviceData: [] };
    }
    const communicationOperatorData = await queryCommunicationOperatorLists(conditions);
    const res = await queryCommunication(conditions);
    const { advice = [], items: data = [] } = res ?? {};
    data.forEach((item: any, index: number) => { item.index = index; });
    data.sort((a: DataType, b: DataType) => b.elapseTime - a.elapseTime);
    return { chartData: wrapChartData(data), analysisChartData: communicationOperatorData, tableData: data, adviceData: advice };
};
const wrapChartData = (data: tableDataType[]): chartDataType => {
    // 显示字段
    const fields = ['rankId', 'startTime', 'elapseTime', 'transitTime', 'synchronizationTime',
        'waitTime', 'synchronizationTimeRatio', 'waitTimeRatio'];
    const chartData: chartDataType = {} as chartDataType;
    fields.forEach(field => {
        chartData[field] = data.map((item: any) => item[field]);
    });
    return chartData;
};

const CommunicationAnalysis = observer(function ({ session, active = true }: { session: Session;active?: boolean }) {
    const [rankId, setRankId] = useState('');
    const [showData, setShowData] = useState<showDataType>({
        chartData: {} as chartDataType,
        analysisChartData: {} as AnalysisChartData,
        tableData: [],
        adviceData: [],
    });
    const [conditions, setConditions] = useState<ConditionDataType>(
        { iterationId: '', rankIds: [], operatorName: '', type: 'CommunicationMatrix', stage: '' });
    const showOperator = (rankId: string): void => {
        setRankId(rankId);
    };
    const returnHome = (): void => { setRankId(''); };
    const handleFilterChange = async(newConditions: ConditionDataType): Promise<void> => {
        setConditions({ ...conditions, ...newConditions });
        const res = await searchData(newConditions);
        setShowData(res);
    };

    const isShow = (name: string): boolean => {
        return conditions.type === name;
    };

    useEffect(() => {
        const inputs = document.querySelectorAll('input');
        inputs.forEach(input => {
            input.setAttribute('maxlength', '200');
        });
    });
    useEffect(() => {
        if (session.durationFileCompleted) {
            search();
        }
        async function search(): Promise<void> {
            if (showData.tableData.length === 0 && conditions.type === 'CommunicationDurationAnalysis') {
                const res = await searchData(conditions);
                setShowData(res);
            }
        }
    }, [session.durationFileCompleted]);

    return <CommunicationAnalysisCom
        session={session}
        handleFilterChange={handleFilterChange} showData={showData}
        active={active} showOperator={showOperator} setShowData={setShowData} conditions={conditions}
        isShow={isShow} rankId={rankId} returnHome={returnHome}
    />;
});

const AdviceLabel = (props: {adviceData: CommunicationAdvice[]}): JSX.Element => {
    const { t } = useTranslation('communication');
    const { adviceData } = props;
    let overAllText = '';
    const issueList: Array<{title: string; content: string }> = [];
    const sdmaData = adviceData.find(item => item.type === 'SDMA');
    const rdmaData = adviceData.find(item => item.type === 'RDMA');
    adviceData.forEach(data => {
        overAllText += t('OverallDuration', { type: data.type, time: data.time });
        // 比较经验带宽（最大带宽的0.8）与平均带宽
        const isBandwidthIssue = data.avg >= data.max * 0.8;
        issueList.push({
            title: data.type,
            content: t('CommunicationAdvice', { ...data, issue: isBandwidthIssue ? t('BandwidthIssue') : t('CommunicationIssue') }),
        });
    });
    if (sdmaData && rdmaData) {
        overAllText += t('MoreFocus', { type: sdmaData.time >= rdmaData.time ? sdmaData.type : rdmaData.type });
    }
    return (
        <div style={{ marginBottom: '20px' }}>
            <div className={'communication-advice-title'}>
                {t('Advice')}
                <StyledTooltip title={
                    (
                        <div style={{ padding: '1rem' }}>
                            {t('AdviceTip')}
                        </div>
                    )
                }>
                    <HelpIcon style={{ cursor: 'pointer', marginLeft: '3px' }} height={20} width={20}/>
                </StyledTooltip>
            </div>
            <div className="communication-advice-header">{t('Overall')}</div>
            <div className="communication-advice-content">{overAllText}</div>
            {
                issueList.map(item => {
                    return (
                        <>
                            <div className="communication-advice-header">{item.title}</div>
                            <div className="communication-advice-content">{item.content}</div>
                        </>
                    );
                })
            }
        </div>
    );
};

const CommunicationAnalysisCom = (props: {isShow:
(name: string) => boolean;session: Session;[propName: string]: any;}): JSX.Element => {
    const {
        session, handleFilterChange, showData, active, showOperator,
        setShowData, conditions, isShow, rankId, returnHome,
    } = props;
    return (
        <div style={{ textAlign: 'left', paddingTop: '20px' }} className={'header-fixed-content-scroll'}>
            {/* 筛选条件 */}
            <Filter handleFilterChange={handleFilterChange} session={session} />
            {/* 通信用时分析 */}
            <Tan
                position={'left'}
                drag={<Help />}
                main={ <div style={{ padding: '0 16px' }}>
                    <CommunicationTimeAnalysisChart dataSource={showData.analysisChartData} session={session}/>
                    <CommunicationTimeChart dataSource={showData.chartData} session={session}/>
                    <CommunicationTimeTable showOperator={showOperator} dataSource={showData.tableData} session={session}
                        conditions={conditions} updateSort={(data): void => {
                            setShowData({ ...showData, chartData: wrapChartData(data) });
                        }}/>
                    { showData.adviceData.length > 0 && <AdviceLabel adviceData={showData.adviceData} /> }
                </div>
                }
                dragSize={400}
                id={'communication-analysis'}
                style={{ display: isShow('CommunicationDurationAnalysis') ? 'block' : 'none' }}
                className={'communication'}
            />
            {/* 通信矩阵 */}
            <CommunicationMatrix isShow={isShow('CommunicationMatrix') && active} conditions={conditions} session={session}/>
            {/* 带宽分析 */}
            { rankId !== '' && <Operators iterationId={conditions.iterationId} rankId={rankId}
                session={session} returnHome={returnHome}
                operatorName={conditions.operatorName} stage={conditions.stage} /> }
        </div>
    );
};

export default CommunicationAnalysis;
