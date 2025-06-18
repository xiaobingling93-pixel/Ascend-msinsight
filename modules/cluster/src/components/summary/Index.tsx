/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
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
import {
    getParallelismPerformanceDataCancelable,
    queryAllConnections,
    slowRankAdvisor,
} from '../../utils/RequestUtils';
import { runInAction } from 'mobx';
import { Communicator, partitionMode } from '../communicatorContainer/ContainerUtils';
import connector from '../../connection';
import { GetSlowRankAdviseRes, IndicatorsItem, PerformanceDataItem } from '../../utils/interface';
import { isEqual } from 'lodash';
import { ExpertLoadBalancingBox } from './expert-load-balancing';
import { ClusterSelect } from '../ClusterSelect';
import { Label } from '../Common';
import { SlowRankTable } from './SlowRankTable';

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
    group: string; // 形式如：{rankId,rankId,...}
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
    dimension: 'ep-dp-pp',
    dpSize: 1,
    ppSize: 1,
    tpSize: 1,
    epSize: 1,
    cpSize: 1,
    moeTpSize: 1,
};

const defaultSlowRankRes = {
    hasSlowRank: false,
    matchSuccess: false,
    topNElements: [],
};

export const Index = observer(({ session, clusterPath }: { session: Session; clusterPath: string }): JSX.Element => {
    const { t } = useTranslation('summary');
    const tips = useHit(true);
    const [isPipeline, setIsPipeline] = useState(false);
    const [activeRankId, setActiveRankId] = useState('');
    const [performanceLoading, setPerformanceLoading] = useState(false);
    const [slowRankData, setSlowRankData] = useState<GetSlowRankAdviseRes>(defaultSlowRankRes);
    const [adviceContent, setAdviceContent] = useState<string[]>([]);
    const [performanceChartConditions, setPerformanceChartConditions] = useState<PerformanceChartConditions>(defaultPerformanceChartConditions);
    const [generateConditions, setGenerateConditions] = useState<GenerateConditions>(defaultGenerateConditions);
    const isDefaultGenerateConditions: boolean = useMemo(() => {
        return isEqual(generateConditions, defaultGenerateConditions);
    }, [generateConditions]);

    const getPerformanceData = async (): Promise<void> => {
        setPerformanceLoading(true);
        const params = {
            step: performanceChartConditions.step,
            baselineStep: session.isCompare ? performanceChartConditions.baselineStep : null,
            ...generateConditions,
            isCompare: session.isCompare,
        };

        const { invoke } = getParallelismPerformanceDataCancelable;
        const { performance, advice } = await invoke(params).finally(() => {
            setPerformanceLoading(false);
        });
        if (performance.length !== 0) {
            const curName = params.dimension === 'ep-dp-pp-cp-tp' ? 'Communication' : 'Avg Communication';
            session.dynamicsIndicatorList = Object.keys(performance[0].commTimeIndicator.diff).map(item => {
                return {
                    key: item,
                    name: curName,
                    unit: 'μs',
                } as IndicatorsItem;
            });
        }
        const performanceAfterDeal = performance.map(item => {
            return {
                index: item.index,
                ...item.indicators.compare,
                diff: item.indicators.diff,
                commCompare: item.commTimeIndicator.compare,
                commDiff: item.commTimeIndicator.diff,
            };
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
        const { connections } = await queryAllConnections({ ...generateConditions, dimension: 'ep-dp-pp-cp-tp', clusterPath });
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
                clusterPath,
                partitionModes,
                defaultPPSize: 0,
            };
        });
        connector.send({ event: 'updateCommunicatorData', body: session.communicatorData, to: 'Communication' });
    };

    const getSlowRankData = async (): Promise<void> => {
        const params = {
            ...generateConditions,
        };
        const slowRankRes: GetSlowRankAdviseRes = await slowRankAdvisor(params);
        setSlowRankData(slowRankRes);
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
        if (isDefaultGenerateConditions) {
            return;
        }
        getPerformanceData();
        getSlowRankData();
    }, [performanceChartConditions.step, performanceChartConditions.baselineStep, JSON.stringify(generateConditions), session.isCompare]);

    useEffect(() => {
        if (isDefaultGenerateConditions) {
            return;
        }
        getAllConnections();
    }, [
        generateConditions.algorithm,
        generateConditions.ppSize,
        generateConditions.tpSize,
        generateConditions.cpSize,
        generateConditions.dpSize,
        generateConditions.epSize,
    ]);

    useEffect(() => {
        setGenerateConditions(defaultGenerateConditions);
    }, [clusterPath]);

    return <Layout>
        <div style={{ display: 'inline-block', height: '30px', lineHeight: '30px', margin: '0 20px 10px 0' }}>
            <Label name={t('Cluster')}/>
            <ClusterSelect width={300} session={session}/>
        </div>

        <BaseInfo session={session}/>

        <CollapsiblePanel title={t('Parallel Strategy Analysis')}>
            <CommunicatorContainer
                session={session}
                generateConditions={generateConditions}
                onGenerateConditionsChange={handleGenerateConditionsChange}
                loading={performanceLoading}
                clusterPath={clusterPath}
            />
            <SlowRankTable
                generateConditions={generateConditions}
                slowRankRes={slowRankData}
            />
            {!isDefaultGenerateConditions && <CollapsiblePanel
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
                                clusterPath={clusterPath}
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
                                    dbPath={session.rankDbPathMap.get(activeRankId) ?? ''}
                                />
                            }
                        </>
                }
            </CollapsiblePanel>
            }
        </CollapsiblePanel>

        <CollapsiblePanel title={t('MoE Expert Load Balancing Analysis')}>
            <ExpertLoadBalancingBox />
        </CollapsiblePanel>
    </Layout>;
});

export default Index;
