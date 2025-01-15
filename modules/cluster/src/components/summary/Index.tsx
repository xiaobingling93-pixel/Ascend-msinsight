/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { ExclamationCircleFilled } from '@ant-design/icons';
import type { Session } from '../../entity/session';
import { useEventBus } from '../../utils/eventBus';
import { Filter } from './Filter';
import { StatisticsTable } from './StatisticsTable';
import BaseInfo from './BaseInfo';
import { CommunicatorContainer, GenerateConditions } from '../communicatorContainer/CommunicatorContainer';
import { HelpIcon } from 'ascend-icon';
import { Layout } from 'ascend-layout';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Tooltip } from 'ascend-components';
import { PerformanceChart } from './PerformanceChart';
import { FlowChart } from './FlowChart';
import styled from '@emotion/styled';
import { getParallelismPerformanceData, queryAllConnections } from '../../utils/RequestUtils';
import { runInAction } from 'mobx';
import { Communicator, partitionMode } from '../communicatorContainer/ContainerUtils';
import connector from '../../connection';
import { PerformanceDataItem } from '../../utils/interface';

const FlowChartContainer = styled.div`
    margin-top: 24px;
    padding: 16px 24px;
    border: 1px solid ${(props): string => props.theme.borderColor};

    .title {
        color: ${(props): string => props.theme.textColorPrimary};
        margin-bottom: 20px;
        font-size: 16px;
        font-weight: 500;
    }
`;

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

export interface PerformanceChartConditions {
    step: string;
    baselineStep?: string;
    orderBy: string;
    top: string;
    group: string;
}

export interface CommunicatorData {
    name: string;
    value: string;
}

const defaultPerformanceChartConditions = {
    step: 'All',
    baselineStep: 'All',
    group: 'All',
    orderBy: 'rankId',
    top: 'All',
};

const defaultGenerateConditions: GenerateConditions = {
    algorithm: 'megatron-lm(tp-cp-ep-dp-pp)',
    dimension: 'ep-dp',
    dpSize: 1,
    ppSize: 1,
    tpSize: 1,
    epSize: 1,
    cpSize: 1,
};

export const Index = observer(({ session }: { session: Session }): JSX.Element => {
    const { t } = useTranslation('summary');
    const tips = useHit(true);
    const [isPipeline, setIsPipeline] = useState(false);
    const [activeRankId, setActiveRankId] = useState('');
    const [performanceLoading, setPerformanceLoading] = useState(true);
    const [adviceContent, setAdviceContent] = useState<string[]>([]);
    const [performanceChartConditions, setPerformanceChartConditions] = useState<PerformanceChartConditions>(defaultPerformanceChartConditions);
    const [generateConditions, setGenerateConditions] = useState<GenerateConditions>(defaultGenerateConditions);

    const getPerformanceData = async (): Promise<void> => {
        setPerformanceLoading(true);
        const params = {
            step: performanceChartConditions.step,
            baselineStep: session.isCompare ? performanceChartConditions.baselineStep : null,
            ...generateConditions,
            isCompare: session.isCompare,
        };
        const { performance, advice } = await getParallelismPerformanceData(params).finally(() => {
            setPerformanceLoading(false);
        });
        const performanceAfterDeal = performance.map(item => {
            return { index: item.index, ...item.indicators.compare, diff: item.indicators.diff };
        });
        setAdviceContent(advice ?? []);
        runInAction(() => {
            if (performanceAfterDeal !== undefined) {
                session.performanceData = performanceAfterDeal;

                const map: Map<number, PerformanceDataItem> = new Map();
                performanceAfterDeal.forEach(item => {
                    map.set(item.index, item);
                });
                session.performanceDataMap = map;

                session.setRankDyeingData();
            }
        });
    };

    // 获取全展开的连线数据
    const getAllConnections = async (): Promise<void> => {
        const { connections } = await queryAllConnections({ ...generateConditions, dimension: 'ep-dp-pp-cp-tp' });
        runInAction(() => {
            if (connections === undefined) {
                return;
            }

            const partitionModesMap: Map<string, Communicator[]> = new Map();
            connections.forEach(connection => {
                const { type, list } = connection;
                if (!partitionModesMap.has(type)) {
                    partitionModesMap.set(type, []);
                }

                partitionModesMap.get(type)?.push({
                    value: `(${list.join(', ')})`,
                });
            });

            const partitionModes: partitionMode[] = Array.from(partitionModesMap, ([key, value]) => (
                { mode: key, communicators: value }
            ));
            session.communicatorData = {
                partitionModes,
                defaultPPSize: 0,
            };
        });
        connector.send({ event: 'updateCommunicatorData', body: session.communicatorData, to: 4 });
    };

    const handleGenerateConditionsChange = async (params: GenerateConditions): Promise<void> => {
        setGenerateConditions(params);
    };

    const handleFilterChange = (conditions: PerformanceChartConditions): void => {
        setPerformanceChartConditions(conditions);
    };

    useEventBus('activeCommunicator', (data): void => {
        const isPP = (data as CommunicatorData)?.name.startsWith('pipeline');
        setIsPipeline(isPP);

        const ppGroup = (data as CommunicatorData)?.value;
        if (ppGroup !== undefined) {
            setPerformanceChartConditions((prevState) => ({ ...prevState, group: ppGroup }));
        }
    });

    useEventBus('resetPerformanceConditions', (): void => {
        setPerformanceChartConditions(defaultPerformanceChartConditions);
    });

    useEffect(() => {
        if (JSON.stringify(generateConditions) === JSON.stringify(defaultGenerateConditions)) {
            return;
        }
        getPerformanceData();
    }, [performanceChartConditions.step, performanceChartConditions.baselineStep, JSON.stringify(generateConditions), session.isCompare]);

    useEffect(() => {
        if (JSON.stringify(generateConditions) === JSON.stringify(defaultGenerateConditions)) {
            return;
        }
        getAllConnections();
    }, [JSON.stringify(generateConditions)]);

    return session.clusterCompleted
        ? <Layout>
            <BaseInfo session={session}/>

            <CollapsiblePanel title={t('Parallel Strategy Analysis')}>
                <CommunicatorContainer
                    session={session}
                    generateConditions={generateConditions}
                    onGenerateConditionsChange={handleGenerateConditionsChange}
                />

                <CollapsiblePanel
                    id="communication-overview-panel"
                    secondary
                    title={<div className={'flex items-center'}>{t('Computation/CommunicationOverview')}{tips}</div>}
                    headerStyle={{ padding: 0 }}
                    contentStyle={{ paddingLeft: 0, paddingRight: 0 }}
                >
                    <Filter
                        session={session}
                        conditions={performanceChartConditions}
                        onFilterChange={handleFilterChange}
                        isPipeline={isPipeline}
                    />
                    {
                        isPipeline
                            ? <FlowChartContainer data-testid="pipeline-chart">
                                <div className="title">{t('Pipeline Parallelism Chart')}</div>
                                <FlowChart
                                    step={performanceChartConditions.step}
                                    stage={performanceChartConditions.group}
                                />
                            </FlowChartContainer>
                            : <>
                                <div data-testid="performance-chart">
                                    <PerformanceChart
                                        session={session}
                                        {...performanceChartConditions}
                                        {...generateConditions}
                                        loading={performanceLoading}
                                        setActiveRankId={setActiveRankId}
                                        advices={adviceContent}
                                    />
                                </div>
                                {
                                    generateConditions.dimension === 'ep-dp-pp-cp-tp' && !session.isCompare &&
                                <StatisticsTable
                                    session={session}
                                    step={performanceChartConditions.step}
                                    rankId={activeRankId}
                                />
                                }
                            </>
                    }
                </CollapsiblePanel>
            </CollapsiblePanel>
        </Layout>
        : <></>;
});

export default Index;
