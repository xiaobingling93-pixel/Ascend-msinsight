/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import { observer } from 'mobx-react';
import { fetchColumnFilterProps, ResizeTable } from '@insight/lib/resize';
import type { ColumnsType } from 'antd/es/table';
import { DragDirection, useDraggableContainer } from '@insight/lib';
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
import jumpToUnitOperator from '../../utils/jumpToUnitOperator';
import { getDetailTimeDisplay } from '../../insight/units/AscendUnit';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { StyledEmpty } from '@insight/lib/utils';
import type { SelectContentViewProps } from './SystemView';

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

interface OverallMetricsTableProps extends SelectContentViewProps {
    session: Session;
    selectedRow?: GetOverallMetricsResultItem | null;
    setSelectedRow: (row: GetOverallMetricsResultItem | null) => void;
}

const OverallMetricsTable = observer(({ bottomHeight, card, session, selectedRow, setSelectedRow }: OverallMetricsTableProps) => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const [tableData, setTableData] = useState<GetOverallMetricsResultItem[]>([]);
    const [loading, setLoading] = useState(false);
    const [page, setPage] = useState(defaultPage);
    const cardPhase = session.units.find((unit) => (unit.metadata as CardMetaData).cardId === card.cardId)?.phase;
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });

    async function getOverallMetricsData(): Promise<void> {
        if (!card || card.cardId === '') {
            return;
        }
        setLoading(true);
        try {
            const rankId = card.cardId;
            const dbPath = card.dbPath;
            const { data, count: total } = await getOverallMetrics({ rankId, dbPath, pageSize: page.pageSize, current: page.current });
            setPage({ ...page, total });
            setTableData(data ?? []);
            setLoading(false);
        } catch (e) {
            setLoading(false);
            setTableData([]);
            setPage(defaultPage);
        }
    }

    useEffect(() => {
        getOverallMetricsData();
        setSelectedRow(null);
    }, [card.cardId]);

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

interface OverallMetricsMoreProps extends SelectContentViewProps {
    selectedRow?: GetOverallMetricsResultItem | null;
}
const OverallMetricsMoreTable = observer(({ card, session, selectedRow, bottomHeight }: OverallMetricsMoreProps) => {
    const [selectedRowId, setSelectedRowId] = useState<string>();
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });

    const { page, setPage, setSorter, setFilters, loading, tableData } = useMetricsMoreUpdater({
        card,
        selectedRow,
    });

    const rowEvents = (record: GetOverallMetricsMoreListResultItem): React.HTMLAttributes<any> => {
        const { id, name, duration, timestamp } = record;
        return {
            onClick: async (): Promise<void> => {
                const res = await queryOneKernel({ rankId: card.cardId, dbPath: card.dbPath, name, timestamp, duration });
                jumpToUnitOperator({
                    ...record,
                    ...res,
                    duration,
                    cardId: card.cardId,
                    dbPath: card.dbPath,
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

export type MetricsMoreUpdaterType = ({ card, selectedRow }: Pick<OverallMetricsMoreProps, 'card' | 'selectedRow'>) => ({
    page: PageType;
    setPage: (args: PageType) => void;
    sorter: SorterResult<GetOverallMetricsMoreListResultItem>;
    setSorter: React.Dispatch<React.SetStateAction<SorterResult<GetOverallMetricsMoreListResultItem>>>;
    setFilters: React.Dispatch<React.SetStateAction<Record<string, FilterValue | null>>>;
    loading: boolean;
    tableData: GetOverallMetricsMoreListResultItem[];
});

const useMetricsMoreUpdater: MetricsMoreUpdaterType = ({ card, selectedRow }) => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter: SorterResult<GetOverallMetricsMoreListResultItem> = { field: 'duration', order: 'descend' };
    const [page, setPage] = useState<PageType>(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [filters, setFilters] = useState<Record<string, FilterValue | null>>({});
    const [loading, setLoading] = useState(false);
    const [tableData, setTableData] = useState<GetOverallMetricsMoreListResultItem[]>([]);
    const requestTrigger = useRef(true);

    async function getMoreListData(): Promise<void> {
        if (!card || card.cardId === '') {
            return;
        }
        setLoading(true);
        const { sameOperatorsDetails, count: total, pageSize, currentPage: current } = await getOverallMetricsMoreList({
            rankId: card.cardId,
            dbPath: card.dbPath,
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
    }, [card?.cardId, selectedRow?.id]);

    return { page, setPage, sorter, setSorter, setFilters, loading, tableData };
};

/**
 * OverallMetrics组件，用于展示总体指标信息。
 *
 * @param {SelectContentViewProps} props - 组件的属性，包含卡片信息等
 * @returns {JSX.Element} 返回一个React元素，根据条件渲染总体指标表格或空状态
 */
export const OverallMetrics = observer((props: SelectContentViewProps) => {
    const { sessionStore } = store;
    // 获取当前活跃的session
    const session = sessionStore.activeSession;
    // 使用拖拽容器，设置拖拽方向为向右，宽度为400
    const [view] = useDraggableContainer({ dragDirection: DragDirection.RIGHT, draggableWH: 400 });
    // 定义并初始化selectedRow状态，用于存储选中的行数据
    const [selectedRow, setSelectedRow] = useState<GetOverallMetricsResultItem | null>();
    const { t } = useTranslation();

    return props.card !== undefined && props.card.cardId !== ''
        ? view({
            mainContainer: session !== undefined
                ? <OverallMetricsTable {...props} selectedRow={selectedRow} setSelectedRow={setSelectedRow}
                    session={session}/>
                : <></>,
            draggableContainer: <StyledMoreCard
                className="moreContainer"
                title={t('Details')}
                bordered={false}>
                <ChartErrorBoundary className={'more-error'}>
                    <MoreContainer>
                        {session && <OverallMetricsMoreTable {...props} session={session} selectedRow={selectedRow}/>}
                    </MoreContainer>
                </ChartErrorBoundary>
            </StyledMoreCard>,
            id: 'overallMetrics',
        })
        : <div style={{ display: 'flex', height: '100%' }}>
            <StyledEmpty style={{ margin: 'auto' }}/>
        </div>;
});
