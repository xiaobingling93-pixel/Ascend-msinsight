/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React from 'react';
import ComputationCommunicationOverview from '../components/analysisSummary/ComputationCommunicationOverview';
import { Session } from '../entity/session';

const AnalysisSummary = ({ session, active }: { session: Session;active: boolean }): JSX.Element => {
    return <ComputationCommunicationOverview session={session} active={active}/>;
};

export default AnalysisSummary;
