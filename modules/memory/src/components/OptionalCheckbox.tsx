/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useState } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { useTranslation } from 'react-i18next';
import { Checkbox } from 'ascend-components';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import { Label } from './Common';
import { MemorySession, MemoryGraphType } from '../entity/memorySession';

const OptionalCheckbox = observer(({ memorySession }: { memorySession: MemorySession }) => {
    const [
        isOnlyShowAllocatedOrReleasedWithinInterval,
        setIsOnlyShowAllocatedOrReleasedWithinInterval,
    ] = useState<boolean>(memorySession.isOnlyShowAllocatedOrReleasedWithinInterval);
    const { t } = useTranslation('memory');

    const onShowPassThroughTimeIntervalDataCheckboxChanged = (value: CheckboxChangeEvent): void => {
        setIsOnlyShowAllocatedOrReleasedWithinInterval(value.target.checked as boolean);
        runInAction(() => {
            memorySession.isOnlyShowAllocatedOrReleasedWithinInterval = value.target.checked as boolean;
        });
    };

    if (memorySession.memoryType === MemoryGraphType.STATIC) {
        return <></>;
    } else {
        return <div className="flex items-center">
            <Label name={t('searchCriteria.Show Allocated or Released Within Interval Data')} />
            <Checkbox
                id="input-onlyShowAllocatedOrReleased"
                checked={isOnlyShowAllocatedOrReleasedWithinInterval}
                onChange={onShowPassThroughTimeIntervalDataCheckboxChanged}
            />
        </div>;
    }
});

export default OptionalCheckbox;
