/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import React from 'react';
import { observer } from 'mobx-react-lite';
import { CurveSession } from '../entity/curveSession';
import DynamicLineChart from './DynamicLineChart';
import { Session } from '../entity/session';

const CurveLineChart = observer(({ session, curveSession, isDark }:
{ session: Session; curveSession: CurveSession; isDark: boolean }) => {
    const renderLineChart = (): JSX.Element => {
        return (<DynamicLineChart session={session} curveSession={curveSession} isDark={isDark}></DynamicLineChart>);
    };

    return (
        <div className="mb-30">
            {renderLineChart()}
        </div>
    );
});

export default CurveLineChart;
