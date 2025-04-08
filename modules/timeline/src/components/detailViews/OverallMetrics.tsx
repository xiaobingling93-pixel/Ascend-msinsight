/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import { observer } from 'mobx-react';
import { fetchColumnFilterProps, ResizeTable } from 'ascend-resize';
import type { ColumnsType } from 'antd/es/table';
import { DragDirection, useDraggableContainer } from 'ascend-use-draggable-container';
import React, { useEffect, useRef, useState } from 'react';
import { ChartErrorBoundary } from '../error/ChartErrorBoundary';
import { MoreContainer, StyledMoreCard } from '../BottomPanel';
import { store } from '../../store';
import { getOverallMetrics, getOverallMetricsMoreList } from '../../api/request';
import type {
    GetOverallMetricsMoreListResultItem,
    GetOverallMetricsResultItem,
} from '../../api/interface';
import type { FilterValue, SorterResult } from 'antd/lib/table/interface';
import { Session } from '../../entity/session';
import { getDefaultColumData, getPageData, PageType, queryOneKernel } from './Common';
import type { CardMetaData } from '../../entity/data';
import { jumpToUnitOperator } from '../../utils';
import { getDetailTimeDisplay } from '../../insight/units/AscendUnit';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { StyledEmpty } from 'ascend-utils';

interface OverallMetricsProps {
    rankId: string;
    bottomHeight: number;
}

export const overallMetricsColumns = (t: TFunction): ColumnsType<GetOverallMetricsResultItem> => [
    { title: t('Category'), dataIndex: 'name', ellipsis: true },
    { title: t('Total Time(us)'), dataIndex: 'totalTime' },
    {
        title: t('Time Ratio'),
        dataIndex: 'ratio',
        render: (text) => text !== null ? `${text}%` : text,
    },
    { title: t('Number'), dataIndex: 'nums' },
    { title: t('Avg(us)'), dataIndex: 'avg' },
    { title: t('Min(us)'), dataIndex: 'min' },
    { title: t('Max(us)'), dataIndex: 'max' },
];

const overallMetricsMoreColumns = (t: TFunction): ColumnsType<GetOverallMetricsMoreListResultItem> => [
    {
        title: t('Name'),
        dataIndex: 'name',
        ellipsis: true,
        ...fetchColumnFilterProps('name', 'Name'),
        onFilter: undefined,
    },
    {
        title: t('Start Time'),
        dataIndex: 'startTime',
        ...getDefaultColumData('startTime'),
    },
    {
        title: t('Duration(ns)'),
        dataIndex: 'duration',
        ...getDefaultColumData('duration'),
    },
];

interface OverallMetricsTableProps extends OverallMetricsProps {
    session: Session;
    selectedRow?: GetOverallMetricsResultItem | null;
    setSelectedRow: (row: GetOverallMetricsResultItem | null) => void;
}

const OverallMetricsTable = observer(({ bottomHeight, rankId, session, selectedRow, setSelectedRow }: OverallMetricsTableProps) => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const [tableData, setTableData] = useState<GetOverallMetricsResultItem[]>([]);
    const [loading, setLoading] = useState(false);
    const [page, setPage] = useState(defaultPage);
    const cardPhase = session.units.find((unit) => (unit.metadata as CardMetaData).cardId === rankId)?.phase;
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });

    async function getOverallMetricsData(): Promise<void> {
        if (!rankId) {
            return;
        }
        setLoading(true);
        try {
            const { data, count: total } = await getOverallMetrics({ rankId, pageSize: page.pageSize, current: page.current }).finally(() => {
                setLoading(false);
                setTableData([]);
            });
            setPage({ ...page, total });
            setTableData(data ?? []);
        } catch (e) {
            setLoading(false);
        }
    }

    useEffect(() => {
        getOverallMetricsData();
        setSelectedRow(null);
    }, [rankId]);

    useEffect(() => {
        if (cardPhase === 'download') {
            getOverallMetricsData();
        }
    }, [cardPhase]);

    return <ResizeTable
        rowKey={'id'}
        dataSource={tableData}
        columns={overallMetricsColumns(t)}
        loading={loading}
        scroll={{ y: bottomHeight - 146 }}
        pagination={getPageData(page, setPage)}
        rowHoverable={false}
        expandable={{}}
        onRow={(record): { onClick: () => void } => ({
            onClick: (): void => {
                if (record.level === 1) {
                    return;
                }
                setSelectedRow(record);
            },
        })}
        rowClassName={(record): string => {
            const cls = [`level-${record.level}`];
            if (selectedRow?.id === record.id) {
                cls.push('selected-row');
            }
            return cls.join(' ');
        }}
    ></ResizeTable>;
});

interface OverallMetricsMoreProps {
    rankId: string;
    session: Session;
    bottomHeight?: number;
    selectedRow?: GetOverallMetricsResultItem | null;
}
const OverallMetricsMoreTable = observer(({ rankId, session, selectedRow, bottomHeight }: OverallMetricsMoreProps) => {
    const [selectedRowId, setSelectedRowId] = useState<string>();
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });

    const { page, setPage, setSorter, setFilters, loading, tableData } = useMetricsMoreUpdater({
        session,
        rankId,
        selectedRow,
    });

    const rowEvents = (record: GetOverallMetricsMoreListResultItem): React.HTMLAttributes<any> => {
        const { id, name, duration, timestamp } = record;
        return {
            onClick: async (): Promise<void> => {
                const res = await queryOneKernel({ rankId, name, timestamp, duration });
                jumpToUnitOperator({
                    ...record,
                    ...res,
                    duration,
                    cardId: rankId,
                    tid: res.threadId,
                    id: res.id,
                });
                setSelectedRowId(id);
            },
        };
    };

    return <ResizeTable
        rowKey={'name'}
        dataSource={tableData}
        columns={overallMetricsMoreColumns(t)}
        scroll={{ y: bottomHeight !== undefined ? bottomHeight - 200 : undefined }}
        loading={loading}
        pagination={getPageData(page, setPage)}
        onRow={(record): React.HTMLAttributes<any> => rowEvents(record)}
        rowClassName={(record): string => {
            return record.id === selectedRowId ? 'selected-row' : 'click-able';
        }}
        onChange={(pagination, filters, newSorter, extra): void => {
            if (extra.action === 'sort') {
                setSorter(newSorter as SorterResult<GetOverallMetricsMoreListResultItem>);
            }
            if (extra.action === 'filter') {
                setFilters(filters);
            }
        }}
    ></ResizeTable>;
});

export type MetricsMoreUpdaterType = ({ session, rankId, selectedRow }: OverallMetricsMoreProps) => ({
    page: PageType;
    setPage: (args: PageType) => void;
    sorter: SorterResult<GetOverallMetricsMoreListResultItem>;
    setSorter: React.Dispatch<React.SetStateAction<SorterResult<GetOverallMetricsMoreListResultItem>>>;
    setFilters: React.Dispatch<React.SetStateAction<Record<string, FilterValue | null>>>;
    loading: boolean;
    tableData: GetOverallMetricsMoreListResultItem[];
});

const useMetricsMoreUpdater: MetricsMoreUpdaterType = ({ rankId, selectedRow }) => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter: SorterResult<GetOverallMetricsMoreListResultItem> = { field: 'duration', order: 'descend' };
    const [page, setPage] = useState<PageType>(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [filters, setFilters] = useState<Record<string, FilterValue | null>>({});
    const [loading, setLoading] = useState(false);
    const [tableData, setTableData] = useState<GetOverallMetricsMoreListResultItem[]>([]);
    const requestTrigger = useRef(true);

    async function getMoreListData(): Promise<void> {
        if (!rankId) {
            return;
        }
        setLoading(true);
        const { sameOperatorsDetails, count: total, pageSize, currentPage: current } = await getOverallMetricsMoreList({
            rankId,
            name: filters.name?.[0] as string | undefined,
            categoryList: selectedRow?.categoryList ?? [],
            order: sorter.order,
            orderBy: sorter.field as keyof GetOverallMetricsMoreListResultItem,
            pageSize: page.pageSize,
            current: page.current,
        }).finally(() => {
            setLoading(false);
            setTableData([]);
        });
        const data = sameOperatorsDetails.map(item => {
            return {
                ...item,
                startTime: getDetailTimeDisplay(item.timestamp),
            };
        });
        setPage({ ...page, pageSize, current, total });
        setTableData(data ?? []);
    }

    useEffect(() => {
        if (selectedRow) {
            getMoreListData();
        } else {
            setTableData([]);
        }
    }, [sorter.field, sorter.order, page.current, page.pageSize, filters.name, requestTrigger.current]);

    useEffect(() => {
        setPage(defaultPage);
        requestTrigger.current = !requestTrigger.current;
    }, [rankId, selectedRow?.id]);

    return { page, setPage, sorter, setSorter, setFilters, loading, tableData };
};

export const OverallMetrics = observer((props: OverallMetricsProps) => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    const [view] = useDraggableContainer({ dragDirection: DragDirection.RIGHT, draggableWH: 400 });
    const [selectedRow, setSelectedRow] = useState<GetOverallMetricsResultItem | null>();
    const { t } = useTranslation();

    return props.rankId !== undefined && props.rankId !== ''
        ? view({
            mainContainer: session !== undefined
                ? <OverallMetricsTable {...props} selectedRow={selectedRow} setSelectedRow={setSelectedRow}
                    session={session}/>
                : <></>,
            draggableContainer: <StyledMoreCard
                className="moreContainer"
                title={t('More')}
                bordered={false}>
                <ChartErrorBoundary className={'more-error'}>
                    <MoreContainer>
                        {session && <OverallMetricsMoreTable session={session} {...props} selectedRow={selectedRow}/>}
                    </MoreContainer>
                </ChartErrorBoundary>
            </StyledMoreCard>,
            id: 'overallMetrics',
        })
        : <div style={{ display: 'flex', height: '100%' }}>
            <StyledEmpty style={{ margin: 'auto' }}/>
        </div>;
});
