/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
