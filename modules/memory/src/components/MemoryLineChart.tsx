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
import { Session } from '../entity/session';
import { MemorySession, MemoryGraphType } from '../entity/memorySession';
import DynamicLineChart from './DynamicLineChart';
import StaticLineChart from './StaticLineChart';

const MemoryLineChart = observer(({ session, memorySession, isDark }:
{ session: Session; memorySession: MemorySession; isDark: boolean }) => {
    const renderLineChart = (): JSX.Element => {
        switch (memorySession.memoryType) {
            case MemoryGraphType.DYNAMIC:
                return (<DynamicLineChart session={session} memorySession={memorySession} isDark={isDark}></DynamicLineChart>);
            case MemoryGraphType.STATIC:
                return (
                    <div>
                        <DynamicLineChart session={session} memorySession={memorySession} isDark={isDark}></DynamicLineChart>
                        <StaticLineChart session={session} memorySession={memorySession} isDark={isDark}></StaticLineChart>
                    </div>
                );
            default:
                return (<DynamicLineChart session={session} memorySession={memorySession} isDark={isDark}></DynamicLineChart>);
        };
    };

    return (
        <div className="mb-30">
            {renderLineChart()}
        </div>
    );
});

export default MemoryLineChart;
