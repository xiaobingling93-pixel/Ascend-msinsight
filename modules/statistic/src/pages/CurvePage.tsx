/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React, { useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { Session } from '../entity/session';
import { useRootStore } from '../context/context';
import { Layout } from 'ascend-layout';
import CurveHeader from '../components/CurveHeader';
import CurveLineChart from '../components/CurveLineChart';
import CurveDetailTable from '../components/CurveDetailTable';
const CurvePage = observer(({ session, isDark }: { session: Session; isDark: boolean }) => {
    const { memoryStore } = useRootStore();
    const curveSession = memoryStore.activeSession;
    useEffect(() => {
        if (!curveSession) {
            return;
        };
    }, [curveSession?.rankIdCondition.value]);

    return (
        curveSession
            ? <Layout>
                <CurveHeader session={session} curveSession={curveSession}></CurveHeader>
                <CurveLineChart curveSession={curveSession} isDark={isDark}></CurveLineChart>
                <CurveDetailTable session={session} curveSession={curveSession}></CurveDetailTable>
            </Layout>
            : <></>
    );
});

export default CurvePage;
