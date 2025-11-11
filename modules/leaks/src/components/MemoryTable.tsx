/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useState } from 'react';
import { type RadioChangeEvent } from 'antd';
import { useTranslation } from 'react-i18next';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import { Radio, Button } from '@insight/lib/components';
import { Session } from '../entity/session';
import BlocksTable from './BlocksTable';
import EventsTable from './EventsTable';
import ThresholdModal from './ThresholdModal';
const MemoryTable = observer(({ session }: { session: Session }): React.ReactElement => {
    const { t } = useTranslation('leaks');
    const { tableType } = session;
    const [open, setOpen] = useState(false);
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
                session.lazyUsedThreshold = { perT: null, valueT: null };
                session.delayedFreeThreshold = { perT: null, valueT: null };
                session.longIdleThreshold = { perT: null, valueT: null };
                session.onlyInefficient = false;
            }
        });
    };
    return (
        <>
            <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                <Radio.Group value={tableType}
                    style={{ marginBottom: 10 }}
                    onChange={radioChange}>
                    <Radio data-testid={'blockViewRadio'} value={'blocks'}>{t('Block View')}</Radio>
                    <Radio
                        data-testid={'eventViewRadio'} value={'events'}>{t('Event View')}</Radio>
                </Radio.Group>
                {tableType === 'blocks' ? <Button type="primary" onClick={() => { setOpen(true); }}>{t('setThreshold')}</Button> : <></>}
            </div>
            {tableType === 'blocks' ? <><BlocksTable session={session} /><ThresholdModal session={session} open={open} setOpen={setOpen} /></> : <EventsTable session={session} />}
        </>
    );
});

export default MemoryTable;
