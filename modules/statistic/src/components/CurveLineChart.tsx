/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React from 'react';
import { observer } from 'mobx-react-lite';
import { CurveSession } from '../entity/curveSession';
import DynamicLineChart from './DynamicLineChart';

const CurveLineChart = observer(({ curveSession, isDark }:
{ curveSession: CurveSession; isDark: boolean }) => {
    const renderLineChart = (): JSX.Element => {
        return (<DynamicLineChart curveSession={curveSession} isDark={isDark}></DynamicLineChart>);
    };

    return (
        <div className="mb-30">
            {renderLineChart()}
        </div>
    );
});

export default CurveLineChart;
