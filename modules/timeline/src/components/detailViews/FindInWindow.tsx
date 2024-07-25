/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import type { Session } from '../../entity/session';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { Button } from 'lib/components';
import { runInAction } from 'mobx';
import { getDefaultColumData, getPageData, searchAllSlices } from './Common';
import ResizeTable from 'lib/ResizeTable';
import { ChartErrorBoundary } from '../error/ChartErrorBoundary';
import styled from '@emotion/styled';
import { RankFilter } from './SystemView';
import { Space } from 'antd/lib/index';
import { getDetailTimeDisplay, ThreadUnit } from '../../insight/units/AscendUnit';
import type { InsightUnit } from '../../entity/insight';
import { colorPalette, getTimeOffset } from '../../insight/units/utils';
import { hashToNumber } from '../../utils/colorUtils';
import type { ThreadMetaData } from '../../entity/data';
import { calculateDomainRange } from '../CategorySearch';

const CONTAINER = styled.div`
    height: calc(100% - 50px);
    padding: 5px 5px 15px 5px;
    .ant-table-wrapper {
        height: 100%;
    }
`;
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
    /**
     *
     * @type {string}
     * @memberof id
     */
    id: string;
    /**
     *
     * @type {string}
     * @memberof tid
     */
    tid: string;
    /**
     *
     * @type {string}
     * @memberof pid
     */
    pid: string;
    /**
     *
     * @type {string}
     * @memberof deviceId
     */
    deviceId: string;
    /**
     *
     * @type {number}
     * @memberof depth
     */
    depth: number;
}

export function useFindDetail(session: Session, bottomHeight: number): any {
    const { t } = useTranslation('timeline');
    return {
        label: <div className={'title'}><span>{t('Find')}</span></div>,
        key: 'Find',
        children: <FindDetailView session={session} bottomHeight={bottomHeight}/>,
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
        <ChartErrorBoundary><FindDetail rankId={conditions.rankId} session={props.session} bottomHeight={props.bottomHeight}>
        </FindDetail></ChartErrorBoundary></>
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
    const [rowData, setRowData] = useState<any>({});
    const [allCondition, setAllCondition] = useState({ doContextSearch: props.session.doContextSearch, page, sorter, selectRank: props.rankId });
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });
    let columns = [];
    columns = [...useColumns(), {
        title: t('Click To Timeline'),
        dataIndex: 'click',
        ellipsis: true,
        render: (_: any, record: any): React.ReactElement => (<Button type="link"
            onClick={(): void => {
                setRowData({ name: record.name ?? record.originOptimizer, ...record });
            }}>{t('Click')}</Button>),
    }];
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
    useEffect(() => {
        if (rowData.name === null || rowData.name === undefined) {
            return;
        }
        handleFindSelected(rowData, props);
    }, [rowData]);
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
        const timestampoffset = getTimeOffset(props.session, props.rankId);
        const data = res.searchAllSlicesDetails.map(item => {
            item.startTime = getDetailTimeDisplay(item.timestamp - timestampoffset);
            return item;
        });
        setDataSource(data);
        setPage({ ...page, total: res.count });
    };
    return <CONTAINER>
        <ResizeTable
            onChange={(pagination: unknown, filters: unknown, newsorter: unknown, extra: {action: string}): void => {
                if (extra.action === 'sort') {
                    setSorter(newsorter as typeof sorter);
                }
            }}
            pagination={getPageData(page, setPage)}
            dataSource={dataSource}
            columns={columns}
            scroll={{ y: props.bottomHeight - 180 }}
            size="small"
            loading = {isLoading}
        />
    </CONTAINER>;
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

const handleFindSelected = async(rowData: any, props: any): Promise<void> => {
    const queryName = rowData.name ?? rowData.originOptimizer;
    runInAction(() => {
        props.session.locateUnit = {
            target: (unit: any): boolean => {
                return unit instanceof ThreadUnit && (Boolean(unit.metadata.cardId === rowData.deviceId)) &&
                unit.metadata.threadId === rowData.tid && unit.metadata.processId === rowData.pid;
            },
            onSuccess: (unit: InsightUnit): void => {
                const startTime = rowData.timestamp - getTimeOffset(props.session, (unit.metadata as ThreadMetaData).cardId);
                const [rangeStart, rangeEnd] = calculateDomainRange(props.session, startTime, rowData.duration);
                props.session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                props.session.selectedData = {
                    id: rowData.id,
                    startTime,
                    name: queryName,
                    color: colorPalette[hashToNumber(rowData.name, colorPalette.length)],
                    duration: rowData.duration,
                    depth: rowData.depth,
                    threadId: rowData.tid,
                    startRecordTime: props.session.startRecordTime,
                    metaType: rowData.pid,
                    showDetail: false,
                };
            },
        };
    });
};
