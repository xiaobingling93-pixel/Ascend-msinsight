/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import type { Session } from '../../entity/session';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { getDefaultColumData, GetPageData, searchAllSlices } from './Common';
import ResizeTable from 'lib/ResizeTable';
import { ChartErrorBoundary } from '../error/ChartErrorBoundary';
import { RankFilter } from './SystemView';
import { Space } from 'antd/lib/index';
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

export function useFindDetail(session: Session): any {
    const { t } = useTranslation('timeline');
    return {
        label: <div className={'title'}><span>{t('Find')}</span></div>,
        key: 'Find',
        children: <FindDetailView session={session}/>,
    };
}

const useColumns = (): any => {
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });
    return [
        { title: t('Name'), dataIndex: 'name', ...getDefaultColumData('name') },
        { title: t('Start Time'), dataIndex: 'startTime', ...getDefaultColumData('startTime') },
        { title: t('Duration(ns)'), dataIndex: 'duration', ...getDefaultColumData('duration') },
    ];
};

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

// eslint-disable-next-line max-lines-per-function
const FindDetail = observer((props: any) => {
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [isLoading, setLoading] = useState(false);
    const [allCondition, setAllCondition] = useState({ doContextSearch: props.session.doContextSearch, page, sorter, selectRank: props.rankId });

    useEffect(() => {
        setAllCondition({ ...allCondition, page, sorter });
    }, [sorter, page.current, page.pageSize]);
    useEffect(() => {
        setAllCondition({ ...allCondition, doContextSearch: props.session.doContextSearch, page: defaultPage, selectRank: props.rankId });
        setPage(defaultPage);
    }, [props.session.doContextSearch, props.rankId]);
    useEffect(() => {
        updateData(allCondition.page, allCondition.sorter, props);
    }, [allCondition.sorter, allCondition.selectRank, allCondition.page.current,
        allCondition.page.pageSize, allCondition.doContextSearch, props.session.doReset]);
    const updateData = async(pages: any, sorters: {field: string;order: string}, prop: any): Promise<void> => {
        if (props.rankId === undefined || props.rankId === '') {
            setDataSource([]);
            setPage(defaultPage);
            setSorter(defaultSorter);
            setAllCondition({ ...allCondition, page: defaultPage, sorter: defaultSorter });
            return;
        }
        if (prop.session.searchData === undefined || prop.session.searchData?.content === '') {
            setDataSource([]);
            setPage(defaultPage);
            setSorter(defaultSorter);
            setAllCondition({ ...allCondition, page: defaultPage, sorter: defaultSorter });
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
            pagination={GetPageData(page, setPage)} dataSource={dataSource} columns={useColumns()} size="small" loading = {isLoading} rowClassName={'click-able'}
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
