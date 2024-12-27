/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React from 'react';
import Index from '../components/summary/Index';
import type { Session } from '../entity/session';

const AnalysisSummary = ({ session }: { session: Session;active?: boolean }): JSX.Element => {
    return <Index session={session} />;
};

export default AnalysisSummary;
