/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import type { RadioChangeEvent } from 'antd';
import { useTranslation } from 'react-i18next';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import { Radio } from 'ascend-components';
import { Session } from '../entity/session';
import BlocksTable from './BlocksTable';
import EventsTable from './EventsTable';

const MemoryTable = observer(({ session }: { session: Session }): React.ReactElement => {
    const { t } = useTranslation('leaks');
    const { tableType } = session;
    const radioChange = (e: RadioChangeEvent): void => {
        runInAction(() => {
            const type = e.target.value;
            session.tableType = type;
            if (type === 'blocks') {
                session.blocksTableData = [];
                session.blocksTableHeader = [];
                session.blocksCurrentPage = 1;
                session.blocksPageSize = 10;
                session.blocksTotal = 0;
                session.blocksOrder = '';
                session.blocksOrderBy = '';
                session.blocksFilters = {};
                session.blocksRangeFilters = {};
            } else {
                session.eventsTableData = [];
                session.eventsTableHeader = [];
                session.eventsCurrentPage = 1;
                session.eventsPageSize = 10;
                session.eventsTotal = 0;
                session.eventsOrder = '';
                session.eventsOrderBy = '';
                session.eventsFilters = {};
                session.eventsRangeFilters = {};
            }
        });
    };
    return (
        <>
            <Radio.Group value={tableType}
                style={{ marginBottom: 10 }}
                onChange={radioChange}>
                <Radio value={'blocks'}>{t('Block View')}</Radio>
                <Radio
                    value={'events'}>{t('Event View')}</Radio>
            </Radio.Group>
            {tableType === 'blocks' ? <BlocksTable session={session} /> : <EventsTable session={session} />}
        </>
    );
});

export default MemoryTable;
