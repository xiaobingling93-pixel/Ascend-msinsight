/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState } from 'react';
import { BaseContainer } from 'lib/CommonUtils';
import { observer } from 'mobx-react';
import { type Session } from '../../../entity/session';
import MemoryChart from './MemoryChart';
import Filter, { defaultCondition, type Icondition } from './Filter';
import MemoryTable from './MemoryTable';

const index = observer(({ session }: { session: Session }): JSX.Element => {
    const [condition, setCondition] = useState<Icondition>(defaultCondition);

    const handleFilterChange = (newCondition: Icondition): void => {
        setCondition({ ...condition, ...newCondition });
    };

    return (
        <BaseContainer
            header="Memory Workload Analysis"
            body={<>
                <Filter blockIdList={session.blockIdList} handleFilterChange={handleFilterChange}/>
                <MemoryChart condition={condition}/>
                <MemoryTable condition={condition}/>
            </>
            }
        />
    );
});

export default index;
