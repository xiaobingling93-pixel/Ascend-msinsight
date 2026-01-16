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
import type { Session } from '../entity/session';
import connector from '../connection';
import { calculateSelectedUnitListStatus } from './actionPinUnits';
import { queryCommunicationKernelDetail } from '../api/request';

async function findInCommunication(session: Session): Promise<void> {
    if (!session.selectedData) {
        return;
    }
    const { name, cardId: rankId, dbPath } = session.selectedData;
    const params = {
        rankId,
        dbPath,
        name,
    };

    const res = await queryCommunicationKernelDetail(params);
    connector.send({
        event: 'switchModule',
        body: {
            switchTo: 'communication',
            toModuleEvent: 'locateCommunication',
            params: {
                operatorName: session.selectedData?.name,
                iterationId: res?.step,
                stage: res?.group,
            },
        },
    });
}

export const actionFindInCommunication = register({
    name: 'findInCommunication',
    label: 'timeline:contextMenu.Find in Communication',
    visible: (session) => {
        const selectedUnitListStatus = calculateSelectedUnitListStatus(session.selectedUnits);
        // session.selectedData 在这里指选中的算子数据
        const isCommunicationOperator = (session.selectedData?.name as string)?.startsWith('hcom_') ?? false;

        return selectedUnitListStatus.isAllThreadNameStartWithGroup && isCommunicationOperator && session.isCluster;
    },
    perform: (session): void => {
        findInCommunication(session);
    },
});
