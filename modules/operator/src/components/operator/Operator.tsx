/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useState } from 'react';
import { observer } from 'mobx-react';
import './Operator.css';
import { Session } from '../../entity/session';
import { HeaderFixedContainer } from '../Common';
import Filter from './Filter';
import DetailChart from './DetailChart';
import BaseTable from './DetailTable';
import { type ConditionType, defaultCondition } from './Filter';

// eslint-disable-next-line max-lines-per-function
const Index = observer(function ({ session }: { session: Session }) {
    const [ condition, setCondition ] = useState<ConditionType>(defaultCondition);
    const handleFilterChange = (obj: any): void => {
        const newCondition = { ...condition, ...obj };
        setCondition(newCondition);
    };

    return <div style={{ height: '100%', width: '100%', overflow: 'auto' }}>
        <HeaderFixedContainer
            style={{ minWidth: '600px' }}
            headerStyle={{ padding: '10px' }}
            header={<Filter session={session} handleFilterChange={handleFilterChange}/>}
            body={ <>
                <DetailChart condition={condition} />
                <BaseTable condition={condition} session={session}/>
            </>
            }
        />
    </div>;
});

export default Index;
