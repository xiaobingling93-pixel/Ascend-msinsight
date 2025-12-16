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
import React, { useState } from 'react';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { type Session } from '../../../entity/session';
import MemoryChart from './MemoryChart';
import Filter, { defaultCondition, type Icondition } from './Filter';
import MemoryTable from './MemoryTable';
import { CollapsiblePanel } from '@insight/lib/components';

const index = observer(({ session }: { session: Session }): JSX.Element => {
    const [condition, setCondition] = useState<Icondition>(defaultCondition);
    const { t: tDetails } = useTranslation('details');
    const handleFilterChange = (newCondition: Icondition): void => {
        setCondition({ ...condition, ...newCondition });
    };

    return (
        <CollapsiblePanel title={tDetails('Memory Workload Analysis')} collapsible>
            <Filter blockIdList={session.blockIdList} handleFilterChange={handleFilterChange} isCompared={session.dirInfo.isCompare}/>
            <MemoryChart condition={condition} session={session}/>
            <MemoryTable condition={condition} session={session}/>
        </CollapsiblePanel>
    );
});

export default index;
