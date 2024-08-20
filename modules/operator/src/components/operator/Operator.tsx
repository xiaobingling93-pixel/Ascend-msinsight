/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import './Operator.css';
import type { Session } from '../../entity/session';
import Filter from './Filter';
import DetailChart from './DetailChart';
import DetailTable from './DetailTable';
import { type ConditionType, defaultCondition, type FilterType, defaultFilterType } from './Filter';
import { Layout } from 'ascend-layout';

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

    return <Layout>
        <div className="mi-search-box">
            <Filter session={session} handleFilterChange={handleFilterChange}/>
        </div>
        {condition.isCompare as boolean ? <></> : <DetailChart condition={condition} session={session}/>}
        <DetailTable condition={condition} filterType={filterType} session={session}/>
    </Layout>;
});

export default Index;
