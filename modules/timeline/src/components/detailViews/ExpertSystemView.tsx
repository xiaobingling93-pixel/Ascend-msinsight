/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

import { observer } from 'mobx-react';
import React from 'react';
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
import { BaseSummary, type BaseSummaryProps, type SelectContentViewProps } from './SystemView';
import { ExpertSummary } from './ExpertSummary';
import { queryExpertAnalysis, queryOperatorDispatch } from '../../api/request';
import type { BaseSummaryRowItemType } from '../../api/interface';
import jumpToUnitOperator from '../../utils/jumpToUnitOperator';

const ExpertAnalysis = observer((props: SelectContentViewProps) => {
    return <ExpertSummary request={queryExpertAnalysis} {...props} />;
});

const AffinityAPI = observer((props: SelectContentViewProps) => {
    return <BaseSummary request={queryAffinityAPI} columns={affinityAPIColumns} {...props} />;
});

const AffinityOptimizer = observer((props: SelectContentViewProps) => {
    return <BaseSummary request={queryAffinityOptimizer} columns={affinityOptimizerColumns} {...props} />;
});

const AICPUOperator = observer((props: SelectContentViewProps) => {
    return <BaseSummary request={queryAICPUOperators} columns={aicpuOperatorColumns} {...props} />;
});

const ACLNNOperator = observer((props: SelectContentViewProps) => {
    return <BaseSummary request={queryACLNNOperators} columns={aclnnOperatorColumns} {...props} />;
});

const FusedOperator = observer((props: SelectContentViewProps) => {
    return <BaseSummary request={queryOperatorFusion} columns={fusionOperatorColumns} {...props} />;
});

const OperatorDispatch = observer((props: SelectContentViewProps) => {
    return <BaseSummary request={queryOperatorDispatch} columns={operatorDispatchColumns} {...props} />;
});

export const handleAdvisorSelected = async (rowData: BaseSummaryRowItemType, props: BaseSummaryProps): Promise<void> => {
    const queryName = rowData.name ?? rowData.originOptimizer ?? '';
    const res = await queryOneKernel({
        rankId: rowData.rankId,
        dbPath: rowData.dbPath,
        name: queryName,
        timestamp: rowData.startTime,
        duration: rowData.duration,
    });
    const depth = rowData.depth > res.depth ? rowData.depth : res.depth;

    jumpToUnitOperator({
        ...rowData,
        id: props.session.isFullDb as boolean ? rowData.id : res.id,
        cardId: rowData.rankId,
        dbPath: rowData.dbPath,
        timestamp: rowData.startTime,
        depth,
        duration: rowData.duration,
        name: queryName,
    });
};

export const ExpertSystemView = [ExpertAnalysis, AffinityAPI, AffinityOptimizer, AICPUOperator, ACLNNOperator, FusedOperator, OperatorDispatch];
