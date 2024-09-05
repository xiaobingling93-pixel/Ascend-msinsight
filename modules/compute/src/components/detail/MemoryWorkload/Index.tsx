/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState } from 'react';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { type Session } from '../../../entity/session';
import MemoryChart from './MemoryChart';
import Filter, { defaultCondition, type Icondition } from './Filter';
import MemoryTable from './MemoryTable';
import CollapsiblePanel from 'ascend-collapsible-panel';

const index = observer(({ session }: { session: Session }): JSX.Element => {
    const [condition, setCondition] = useState<Icondition>(defaultCondition);
    const { t: tDetails } = useTranslation('details');
    const handleFilterChange = (newCondition: Icondition): void => {
        setCondition({ ...condition, ...newCondition });
    };

    return (
        <CollapsiblePanel title={tDetails('Memory Workload Analysis')}>
            <Filter blockIdList={session.blockIdList} handleFilterChange={handleFilterChange} isCompared={session.dirInfo.isCompare}/>
            <MemoryChart condition={condition} session={session}/>
            <MemoryTable condition={condition} session={session}/>
        </CollapsiblePanel>
    );
});

export default index;
