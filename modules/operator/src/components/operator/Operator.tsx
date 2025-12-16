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
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import './Operator.css';
import type { Session } from '../../entity/session';
import Filter from './Filter';
import DetailChart from './DetailChart';
import DetailTable from './DetailTable';
import { type ConditionType, defaultCondition, type FilterType, defaultFilterType } from './Filter';
import { Layout } from '@insight/lib/components';

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
