/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
    const { name, cardId: rankId } = session.selectedData;
    const params = {
        rankId,
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
