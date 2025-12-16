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

import React, { useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { Session } from '../entity/session';
import { useRootStore } from '../context/context';
import { Layout } from '@insight/lib/components';
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
                <CurveLineChart session={session} curveSession={curveSession} isDark={isDark}></CurveLineChart>
                <CurveDetailTable session={session} curveSession={curveSession}></CurveDetailTable>
            </Layout>
            : <></>
    );
});

export default CurvePage;
