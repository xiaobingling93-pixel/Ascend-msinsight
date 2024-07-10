/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import './Operator.css';
import { Session } from '../../entity/session';
import { HeaderFixedContainer } from '../Common';
import Filter from './Filter';
import DetailChart from './DetailChart';
import BaseTable from './DetailTable';
import { type ConditionType, defaultCondition, type FilterType, defaultFilterType } from './Filter';

// eslint-disable-next-line max-lines-per-function
const Index = observer(({ session }: { session: Session }) => {
    const [condition, setCondition] = useState<ConditionType>(defaultCondition);
    const [filterType] = useState<FilterType>(defaultFilterType);
    const handleFilterChange = (obj: any): void => {
        const newCondition = { ...condition, ...obj };
        setCondition(newCondition);
    };

    useEffect(() => {
        const inputs = document.querySelectorAll('input');
        inputs.forEach(input => {
            input.setAttribute('maxlength', '200');
        });
    }, []);

    return <div style={{ height: '100%', width: '100%', overflow: 'auto' }}>
        <HeaderFixedContainer
            style={{ minWidth: '800px' }}
            headerStyle={{ padding: '10px' }}
            header={<Filter session={session} handleFilterChange={handleFilterChange}/>}
            bodyStyle={{ overflow: 'visible' }}
            body={ <>
                <DetailChart condition={condition} session={session}/>
                <BaseTable condition={condition} filterType={filterType} session={session}/>
            </>
            }
        />
    </div>;
});

export default Index;
