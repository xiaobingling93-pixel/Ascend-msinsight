/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Tooltip } from 'ascend-components';
import type { Session } from '../../entity/session';
import Filter, { AnalysisType, defaultCondition } from './Filter';
import type { ConditionDataType } from './Filter';
import CommunicationTimeTable from './CommunicationTimeTable';
import CommunicationTimeChart, { type DataItem } from './CommunicationTimeChart';
import CommunicationMatrix from './CommunicationMatrix';
import { notNullObj } from '../Common';
import { queryCommunication, queryCommunicationOperatorLists } from '../../utils/RequestUtils';
import CommunicationTimeAnalysisChart from './CommunicationTimeAnalysisChart';
import type { AnalysisChartData } from './CommunicationTimeAnalysisChart';
import { HelpIcon } from 'ascend-icon';
import { Layout } from 'ascend-layout';
import AdviceLabel, { type CommunicationAdvice } from './CommunicationDuration/AdviceLabel';
import Operators from './CommunicationDuration/Opertators';

interface showDataType {
    chartData: [];
    analysisChartData: AnalysisChartData;
    tableData: [];
    adviceData: CommunicationAdvice[];
}

const searchData = async (conditions: ConditionDataType & {isCompare: boolean}): Promise<showDataType> => {
    const notNullKeys: string[] = ['stage', 'operatorName', 'type'];
    if (!notNullObj(conditions, notNullKeys)) {
        return { chartData: [], analysisChartData: { minTime: 0, maxTime: 0, data: [] }, tableData: [], adviceData: [] };
    }
    const communicationOperatorData = await queryCommunicationOperatorLists({ ...conditions });
    const res = await queryCommunication(conditions);
    const { advice = [], items: data = [] } = res ?? {};
    // 默认按总时间降序排序
    data.sort((a: DataItem, b: DataItem) => conditions.isCompare
        ? b.compareData.diff.elapseTime - a.compareData.diff.elapseTime
        : b.compareData.compare.elapseTime - a.compareData.compare.elapseTime);
    return { analysisChartData: communicationOperatorData, tableData: data, chartData: data, adviceData: advice };
};

const CommunicationAnalysis = observer(({ session, active = true }: { session: Session;active?: boolean }) => {
    const [rankId, setRankId] = useState('');
    const [showData, setShowData] = useState<showDataType>({
        chartData: [],
        analysisChartData: {} as AnalysisChartData,
        tableData: [],
        adviceData: [],
    });
    const [conditions, setConditions] = useState<ConditionDataType>(defaultCondition);
    const showOperator = (newRankId: string): void => {
        setRankId(newRankId);
    };
    const returnHome = (): void => { setRankId(''); };
    const handleFilterChange = async(newConditions: ConditionDataType): Promise<void> => {
        setConditions({ ...conditions, ...newConditions });
        if (newConditions.type !== AnalysisType.COMMUNICATION_DURATION_ANALYSIS) {
            return;
        }
        const res = await searchData({ ...newConditions, isCompare: session.isCompare });
        setShowData(res);
    };

    useEffect(() => {
        if (session.durationFileCompleted) {
            search();
        }
        async function search(): Promise<void> {
            if (showData.tableData.length === 0 && conditions.type === AnalysisType.COMMUNICATION_DURATION_ANALYSIS) {
                const res = await searchData({ ...conditions, isCompare: session.isCompare });
                setShowData(res);
            }
        }
    }, [session.durationFileCompleted]);

    return <CommunicationAnalysisCom
        session={session}
        handleFilterChange={handleFilterChange} showData={showData}
        active={active} showOperator={showOperator} setShowData={setShowData} conditions={conditions}
        rankId={rankId} returnHome={returnHome}
    />;
});

const CommunicationAnalysisCom = (props: {[propName: string]: any}): JSX.Element => {
    const {
        session, handleFilterChange, showData, active, showOperator,
        setShowData, conditions, rankId, returnHome,
    } = props;
    const { t } = useTranslation('communication');
    return (
        <Layout>
            {/* 筛选条件 */}
            <Filter handleFilterChange={handleFilterChange} session={session} />
            {/* 通信用时分析 */}
            <div className={'communication'} style={{ display: conditions.type === AnalysisType.COMMUNICATION_DURATION_ANALYSIS ? 'block' : 'none' }}>
                <div>
                    <CommunicationTimeAnalysisChart dataSource={showData.analysisChartData} session={session}/>
                    <CommunicationTimeChart dataSource={showData.chartData} session={session}/>
                    <CommunicationTimeTable showOperator={showOperator} dataSource={showData.tableData} session={session}
                        conditions={conditions} updateSort={(data): void => {
                            setShowData({ ...showData, chartData: data });
                        }}/>
                    { showData.adviceData.length > 0 && <AdviceLabel adviceData={showData.adviceData} /> }
                </div>
            </div>
            {/* 通信矩阵 */}
            <CommunicationMatrix isShow={conditions.type === AnalysisType.COMMUNICATION_MATRIX && active} conditions={conditions} session={session}/>
            {/* 带宽分析 */}
            { rankId !== '' && <Operators iterationId={conditions.iterationId} rankId={rankId}
                session={session} returnHome={returnHome}
                operatorName={conditions.operatorName} stage={conditions.stage} /> }
            <div style={{ position: 'absolute', top: '15px', right: '20px' }}>
                <Tooltip
                    placement="left"
                    title={
                        (
                            <div style={{ padding: '10px' }}>
                                {t('Tooltip')}
                            </div>
                        )
                    }>
                    <HelpIcon style={{ cursor: 'pointer', marginLeft: '3px' }} height={20} width={20}/>
                </Tooltip>
            </div>
        </Layout>
    );
};

export default CommunicationAnalysis;
