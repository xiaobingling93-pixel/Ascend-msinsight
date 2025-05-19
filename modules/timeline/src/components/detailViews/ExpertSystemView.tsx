/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

import { observer } from 'mobx-react';
import React from 'react';
import { runInAction } from 'mobx';
import {
    queryOneKernel,
    queryAffinityOptimizer,
    affinityOptimizerColumns,
    queryAICPUOperators,
    queryACLNNOperators,
    aicpuOperatorColumns,
    aclnnOperatorColumns,
    queryAffinityAPI,
    affinityAPIColumns,
    queryOperatorFusion,
    fusionOperatorColumns,
    operatorDispatchColumns,
} from './Common';
import type { ThreadMetaData } from '../../entity/data';
import { calculateDomainRange } from '../CategorySearch';
import { ThreadUnit } from '../../insight/units/AscendUnit';
import type { InsightUnit } from '../../entity/insight';
import { hashToNumber } from '../../utils/colorUtils';
import { colorPalette, getTimeOffset } from '../../insight/units/utils';
import { BaseSummary } from './SystemView';
import { ExpertSummary } from './ExpertSummary';
import { queryExpertAnalysis, queryOperatorDispatch } from '../../api/request';

const ExpertAnalysis = observer((props: any) => {
    return <ExpertSummary request={queryExpertAnalysis} {...props} />;
});

const AffinityAPI = observer((props: any) => {
    return <BaseSummary request={queryAffinityAPI} columns={affinityAPIColumns} {...props} />;
});

const AffinityOptimizer = observer((props: any) => {
    return <BaseSummary request={queryAffinityOptimizer} columns={affinityOptimizerColumns} {...props} />;
});

const AICPUOperator = observer((props: any) => {
    return <BaseSummary request={queryAICPUOperators} columns={aicpuOperatorColumns} {...props} />;
});

const ACLNNOperator = observer((props: any) => {
    return <BaseSummary request={queryACLNNOperators} columns={aclnnOperatorColumns} {...props} />;
});

const FusedOperator = observer((props: any) => {
    return <BaseSummary request={queryOperatorFusion} columns={fusionOperatorColumns} {...props} />;
});

const OperatorDispatch = observer((props: any) => {
    return <BaseSummary request={queryOperatorDispatch} columns={operatorDispatchColumns} {...props} />;
});

export const handleAdvisorSelected = async(rowData: any, props: any): Promise<void> => {
    const queryName = rowData.name ?? rowData.originOptimizer;
    const nsDuration = Number((rowData.duration * 1000).toFixed(0));
    const res = await queryOneKernel({
        rankId: rowData.rankId,
        name: queryName,
        timestamp: rowData.startTime,
        duration: nsDuration,
    });
    const depth = rowData.depth > res.depth ? rowData.depth : res.depth;
    runInAction(() => {
        props.session.locateUnit = {
            target: (unit: any): boolean => {
                return unit instanceof ThreadUnit && unit.metadata.cardId === rowData.rankId &&
                    unit.metadata.threadId === rowData.tid && unit.metadata.processId === rowData.pid;
            },
            onSuccess: (unit: InsightUnit): void => {
                const startTime = rowData.startTime - getTimeOffset(props.session, unit.metadata as ThreadMetaData);
                const [rangeStart, rangeEnd] = calculateDomainRange(props.session, startTime, nsDuration);
                props.session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                props.session.selectedData = {
                    id: props.session.isFullDb as boolean ? rowData.id : res.id,
                    startTime,
                    name: queryName,
                    color: colorPalette[hashToNumber(queryName, colorPalette.length)],
                    duration: nsDuration,
                    metaType: rowData.pid,
                    threadId: rowData.tid,
                    processId: rowData.pid,
                    cardId: rowData.rankId,
                    startRecordTime: props.session.startRecordTime,
                    showDetail: false,
                    depth,
                };
            },
        };
    });
};

export const ExpertSystemView = [ExpertAnalysis, AffinityAPI, AffinityOptimizer, AICPUOperator, ACLNNOperator, FusedOperator, OperatorDispatch];
