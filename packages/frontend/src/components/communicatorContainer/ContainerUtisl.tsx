/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { communicatorContainerData } from './CommunicatorContainer';

export const getPpContainerData = (data: communicatorContainerData, mode: string): any[] => {
    const tmp = data.partitionModes.find(vl => vl.mode === mode);
    if (tmp === undefined) {
        return [];
    }
    return tmp?.communicators.map(item => ({
        value: item.value, label: item.value,
    }));
};
