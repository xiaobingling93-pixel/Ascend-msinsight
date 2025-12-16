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

import { register } from './register';
import { Session } from '../entity/session';
import { createCurve } from '../api/request';
import connector from '../connection';

async function findLineChart(session: Session, type: string): Promise<void> {
    if (!session.selectedData) {
        return;
    }
    const { name, cardId, processId, threadId } = session.selectedData;
    const params = {
        fileId: cardId as string,
        pid: processId,
        tid: threadId,
        x: name,
        type,
    };
    const res = await createCurve(params);
    connector.send({
        event: 'switchModule',
        body: {
            switchTo: 'statistic',
            toModuleEvent: 'locateGroup',
            params: {
                fileId: cardId as string,
                group: res.curveName,
            },
        },
    });
}

function checkCardIsIE(session: Session): boolean {
    const { isIE, isCluster, selectedData, units } = session;
    const cluster = units.find(unit => unit.metadata?.cardId === selectedData?.cardId)?.metadata?.cluster;
    return Boolean(isIE && selectedData && (!isCluster || (isCluster && !cluster)));
}

export const actionGenerateBubbleCurve = register({
    name: 'generateBubbleCurveByBlock',
    label: 'timeline:contextMenu.Generate Bubble Line Chart By Block',
    visible: (session) => checkCardIsIE(session),
    perform: (session): void => {
        findLineChart(session, '2');
    },
});
export const actionGenerateCurve = register({
    name: 'generateCurveByBlock',
    label: 'timeline:contextMenu.Generate Line Chart By Block',
    visible: (session) => checkCardIsIE(session),
    perform: (session): void => {
        findLineChart(session, '1');
    },
});
