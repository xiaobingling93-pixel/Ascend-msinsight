/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import type { Session } from '../../entity/session';
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import { getDefaultColumData, GetPageData, searchAllSlices } from './Common';
import ResizeTable from 'lib/ResizeTable';
import { ChartErrorBoundary } from '../error/ChartErrorBoundary';
import { RankFilter } from './SystemView';
import { Space } from 'antd/lib/index';
import type { CardMetaData } from '../../entity/data';
import { getDetailTimeDisplay } from '../../insight/units/AscendUnit';

export interface SearchTableData {
    /**
     *
     * @type SearchAllSlicesDetails[]
     * @memberof searchAllSlicesDetails
     */
    searchAllSlicesDetails: SearchAllSlicesDetails[];
    /**
     *
     * @type {number}
     * @memberof count
     */
    count: number;
}

export interface SearchAllSlicesDetails {
    startTime: string;
    /**
     *
     * @type {string}
     * @memberof name
     */
    name: string;
    /**
     *
     * @type {number}
     * @memberof timestamp
     */
    timestamp: number;
    /**
     *
     * @type {number}
     * @memberof duration
     */
    duration: number;
}

export function getFindDetail(session: Session): any {
    return {
        label: <div className={'title'}><span>Find</span></div>,
        key: 'Find',
        children: <FindDetailView session={session}/>,
    };
}

const colums = [
    { title: 'Name', dataIndex: 'name', ...getDefaultColumData('name') },
    { title: 'Start', dataIndex: 'startTime', ...getDefaultColumData('startTime') },
    { title: 'Duration(ns)', dataIndex: 'duration', ...getDefaultColumData('duration') },
];

export const FindDetailView = observer((props: any) => {
    const [conditions, setConditions] = useState<{ rankId: string }>({ rankId: '' });
    const handleChange = (rankId: string): void => {
        setConditions({ rankId });
    };
    return (
        <><Space direction="vertical" size="middle" style={{ display: 'flex' }}>
            <RankFilter session={props.session} handleChange={handleChange}></RankFilter>
        </Space>
        <ChartErrorBoundary><FindDetail rankId={conditions.rankId} session={props.session}></FindDetail></ChartErrorBoundary></>
    );
});

const defaultPage = { current: 1, pageSize: 10, total: 0 };
const defaultSorter = { field: 'duration', order: 'descend' };

const FindDetail = observer((props: any) => {
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [isLoading, setLoading] = useState(false);
    const status = props.session.units.find((unit: any) => (unit.metadata as CardMetaData).cardId === props.rankId)?.phase;

    useEffect(() => {
        updateData(page, sorter, props);
    }, [sorter, props.rankId, page.current, page.pageSize, props.session.doContextSearch]);

    useEffect(() => {
        if (status === 'download') {
            updateData(page, sorter, props);
        }
    }, [status]);
    const updateData = async(pages: any, sorters: {field: string;order: string}, prop: any): Promise<void> => {
        if (props.rankId === undefined || props.rankId === '') {
            setDataSource([]);
            setPage(defaultPage);
            return;
        }
        if (prop.session.searchData === undefined || prop.session.searchData?.content === '') {
            setDataSource([]);
            setPage(defaultPage);
            return;
        }
        setLoading(true);
        const res = await searchData(pages, sorters, prop).finally(() => setLoading(false));
        const data = res.searchAllSlicesDetails.map(item => {
            item.startTime = getDetailTimeDisplay(item.timestamp);
            return item;
        });
        setDataSource(data);
        setPage({ ...page, total: res.count });
    };
    return <div style={{ height: '100%', overflow: 'auto', padding: '5px 5px 15px 5px' }}>
        <ResizeTable
            onChange={(pagination: unknown, filters: unknown, newsorter: unknown, extra: {action: string}): void => {
                if (extra.action === 'sort') {
                    setSorter(newsorter as typeof sorter);
                }
            }}
            pagination={GetPageData(page, setPage)}
            dataSource={dataSource}
            columns={colums}
            size="small"
            loading = {isLoading}
            rowClassName={'click-able'}
        />
    </div>;
});

const searchData = async(pages: any, sorters: {field: string;order: string}, prop: any): Promise<SearchTableData> => {
    const res = await searchAllSlices({
        rankId: prop.rankId,
        pageSize: pages.pageSize,
        current: pages.current,
        orderBy: sorters.field === 'startTime' ? 'timestamp' : sorters.field ?? defaultSorter.field,
        order: sorters.order ?? defaultSorter.order,
        searchContent: prop.session.searchData.content,
        isMatchCase: prop.session.searchData.isMatchCase,
        isMatchExact: prop.session.searchData.isMatchExact,
    });
    return res;
};
