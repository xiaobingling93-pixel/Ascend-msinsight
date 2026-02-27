/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

import React, { useEffect, useRef, useState } from 'react';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import type { ColumnsType } from 'antd/es/table';
import type { FilterValue, SorterResult } from 'antd/lib/table/interface';
import _ from 'lodash';
import { ResizeTable } from '@insight/lib/resize';
import { DragDirection, useDraggableContainer } from '@insight/lib';
import { StyledEmpty } from '@insight/lib/utils';
import { ChartErrorBoundary } from '../error/ChartErrorBoundary';
import { MoreContainer, StyledMoreCard } from '../BottomPanel';
import { store } from '../../store';
import { getMemcpyOverallMetrics, getMemcpyOverallMetricsMoreList } from '../../api/request';
import type {
    GetOverallMetricsMoreListResultItem,
    GetMemcpyOverallResultItem,
} from '../../api/interface';
import { Session } from '../../entity/session';
import { getDefaultColumData, getPageData, PageType, queryOneKernel } from './Common';
import type { CardMetaData } from '../../entity/data';
import jumpToUnitOperator from '../../utils/jumpToUnitOperator';
import { getDetailTimeDisplay } from '../../insight/units/AscendUnit';
import type { SelectContentViewProps } from './SystemView';
import { getTimeOffset } from '../../insight/units/utils';

export const memcpyOverallColumns = (t: TFunction): ColumnsType<GetMemcpyOverallResultItem> => [
    { title: t('Category'), dataIndex: 'name', ellipsis: true, width: 120 },
    { title: t('Total Time(ns)'), dataIndex: 'totalTime' },
    { title: t('Total Size(B)'), dataIndex: 'totalSize' },
    { title: t('Number'), dataIndex: 'number' },
    { title: t('Avg Time(ns)'), dataIndex: 'avgTime' },
    { title: t('Min Time(ns)'), dataIndex: 'minTime' },
    { title: t('Max Time(ns)'), dataIndex: 'maxTime' },
    { title: t('Avg Size(B)'), dataIndex: 'avgSize' },
    { title: t('Min Size(B)'), dataIndex: 'minSize' },
    { title: t('Max Size(B)'), dataIndex: 'maxSize' },
];

const memcpyOverallMetricsMoreColumns = (t: TFunction): ColumnsType<GetOverallMetricsMoreListResultItem> => [
    {
        title: t('Name'),
        dataIndex: 'name',
        ellipsis: true,
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
    {
        title: t('Size(B)'),
        dataIndex: 'size',
        ...getDefaultColumData('size'),
    },
];

interface MemcpyOverallMetricsTableProps extends SelectContentViewProps {
    session: Session;
    selectedRow?: GetMemcpyOverallResultItem | null;
    setSelectedRow: (row: GetMemcpyOverallResultItem | null) => void;
}

const DEFAULT_PAGE = { current: 1, pageSize: 10, total: 0 };
const MemcpyOverallMetricsTable = observer(({ bottomHeight, card, session, selectedRow, setSelectedRow }: MemcpyOverallMetricsTableProps) => {
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });
    const [tableData, setTableData] = useState<GetMemcpyOverallResultItem[]>([]);
    const [loading, setLoading] = useState(false);
    const [page, setPage] = useState<PageType>(DEFAULT_PAGE);
    const cardPhase = React.useMemo(() => {
        return session.units.find((unit) => (unit.metadata as CardMetaData).cardId === card.cardId)?.phase;
    }, [session.units, card.cardId]);

    const getMemcpyOverallMetricsData = React.useCallback(async ({
        startTime, endTime, timestampoffset,
    }: { startTime: number; endTime: number; timestampoffset: number }): Promise<void> => {
        if (!card || card.cardId === '') {
            return;
        }
        setLoading(true);
        try {
            const params = {
                rankId: card.cardId,
                dbPath: card.dbPath,
                pageSize: page.pageSize,
                current: page.current,
                startTime: Math.floor(startTime + timestampoffset),
                endTime: Math.ceil(endTime + timestampoffset),
            };
            const res = await getMemcpyOverallMetrics(params);
            if (res !== undefined) {
                const { data, count: total } = res;
                setPage({ ...page, total });
                setTableData(data ?? []);
            }
        } catch (e) {
            setTableData([]);
            setPage(DEFAULT_PAGE);
        } finally {
            setLoading(false);
        }
    }, [card.cardId, card.dbPath, page.pageSize, page.current]);

    useEffect(() => {
        setSelectedRow(null);
    }, [card.cardId]);

    // 1. 保存最新请求函数引用（解决闭包过期问题）
    const latestFetchRef = useRef(getMemcpyOverallMetricsData);
    useEffect(() => {
        latestFetchRef.current = getMemcpyOverallMetricsData;
    }, [getMemcpyOverallMetricsData]);

    // 2. 创建稳定防抖函数（useMemo + cancel 清理）
    const debouncedFetch = React.useMemo(() => {
        return _.debounce((params: Parameters<typeof getMemcpyOverallMetricsData>[0]) => {
            latestFetchRef.current(params);
        }, 100);
    }, []);

    // 3. 触发请求 + 组件卸载/条件变化时清理
    useEffect(() => {
        if (cardPhase !== 'download') {
            debouncedFetch.cancel(); // 非 download 状态立即取消待执行请求
            return;
        }

        // 参数计算（保持原逻辑）
        const startTime = Math.max(session.timeAnalysisRange?.[0] ?? 0, 0);
        const endTime = Math.max(session.timeAnalysisRange?.[1] ?? 0, 0);
        const timestampoffset = getTimeOffset(session, card);

        debouncedFetch({ startTime, endTime, timestampoffset });

        return () => {
            debouncedFetch.cancel(); // 清理：取消待执行请求，避免内存泄漏/无效 setState
        };
    }, [cardPhase, session.timeAnalysisRange, debouncedFetch, card]);

    return <ResizeTable
        rowKey={'rowKey'}
        dataSource={tableData}
        columns={memcpyOverallColumns(t)}
        loading={loading}
        scroll={{ y: bottomHeight - 146 }}
        pagination={getPageData(page, setPage)}
        rowHoverable={false}
        expandable={{}}
        onRow={(record): { onClick: () => void } => ({
            onClick: (): void => {
                setSelectedRow(record);
            },
        })}
        // 给表格添加类名
        rowClassName={(record): string => {
            // 检查selectedRow的rowKey是否与当前记录的rowKey相同
            return selectedRow?.rowKey === record.rowKey ? 'selected-row' : '';
        }}
    ></ResizeTable>;
});

interface MemcpyOverallMetricsMoreProps extends SelectContentViewProps {
    selectedRow?: GetMemcpyOverallResultItem | null;
}
const MemcpyOverallMetricsMoreTable = observer(({ card, session, selectedRow, bottomHeight }: MemcpyOverallMetricsMoreProps) => {
    const [selectedRowId, setSelectedRowId] = useState<string>();
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });

    const { page, setPage, setSorter, setFilters, loading, tableData } = useMetricsMoreUpdater({
        session,
        card,
        selectedRow,
    });

    const rowEvents = (record: GetOverallMetricsMoreListResultItem): React.HTMLAttributes<any> => {
        const { id, name, duration, timestamp } = record;
        return {
            onClick: async (): Promise<void> => {
                /// FIX: queryOneKernel 接口当查询到的算子有相同的 name 和 timestamp 时，默认返回最后一个算子的信息，这可能导致跳转到错误的算子上。
                /// 因此先取出 selectedRow 的 categoryList 中的 tid 和 record 中的算子 id 进行过滤
                /// 如果两个值不存在，则仍然使用 queryOneKernel 返回的算子 tid 和 id 进行跳转。
                const selectedTid = selectedRow?.categoryList[0];
                const res = await queryOneKernel({ rankId: card.cardId, dbPath: card.dbPath, name, timestamp, duration });
                jumpToUnitOperator({
                    ...record,
                    ...res,
                    duration,
                    cardId: card.cardId,
                    dbPath: card.dbPath,
                    tid: selectedTid ?? res.threadId,
                    id: id ?? res.id,
                });
                setSelectedRowId(id);
            },
        };
    };

    return <ResizeTable
        rowKey={'id'}
        dataSource={tableData}
        columns={memcpyOverallMetricsMoreColumns(t)}
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

export type MetricsMoreUpdaterType = ({ session, card, selectedRow }: Pick<MemcpyOverallMetricsMoreProps, 'session' | 'card' | 'selectedRow'>) => ({
    page: PageType;
    setPage: (args: PageType) => void;
    sorter: SorterResult<GetOverallMetricsMoreListResultItem>;
    setSorter: React.Dispatch<React.SetStateAction<SorterResult<GetOverallMetricsMoreListResultItem>>>;
    setFilters: React.Dispatch<React.SetStateAction<Record<string, FilterValue | null>>>;
    loading: boolean;
    tableData: GetOverallMetricsMoreListResultItem[];
});

const useMetricsMoreUpdater: MetricsMoreUpdaterType = ({ session, card, selectedRow }) => {
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
        let startTime = session.timeAnalysisRange?.[0] ?? 0;
        startTime = startTime < 0 ? 0 : startTime;
        let endTime = session.timeAnalysisRange?.[1] ?? 0;
        endTime = endTime < 0 ? 0 : endTime;
        const timestampoffset = getTimeOffset(session, card);
        const { sameOperatorsDetails, count: total, pageSize, currentPage: current } = await getMemcpyOverallMetricsMoreList({
            rankId: card.cardId,
            dbPath: card.dbPath,
            name: filters.name?.[0] as string | undefined,
            categoryList: selectedRow?.categoryList ?? [],
            order: sorter.order,
            orderBy: sorter.field as keyof GetOverallMetricsMoreListResultItem,
            pageSize: page.pageSize,
            current: page.current,
            startTime: Math.floor(startTime + timestampoffset),
            endTime: Math.ceil(endTime + timestampoffset),
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
    }, [sorter.field, sorter.order, page.current, page.pageSize, filters.name, requestTrigger.current, session.timeAnalysisRange]);

    useEffect(() => {
        setPage(defaultPage);
        requestTrigger.current = !requestTrigger.current;
    }, [card?.cardId, selectedRow?.rowKey]);

    return { page, setPage, sorter, setSorter, setFilters, loading, tableData };
};

/**
 * MemcpyOverallMetrics组件，用于展示内存拷贝总体指标信息。
 *
 * @param {SelectContentViewProps} props - 组件的属性，包含卡片信息等
 * @returns {JSX.Element} 返回一个React元素，根据条件渲染总体指标表格或空状态
 */
export const MemcpyOverallMetrics = observer((props: SelectContentViewProps) => {
    const { sessionStore } = store;
    // 获取当前活跃的session
    const session = sessionStore.activeSession;
    // 使用拖拽容器，设置拖拽方向为向右，宽度为400
    const [view] = useDraggableContainer({ dragDirection: DragDirection.RIGHT, draggableWH: 400 });
    // 定义并初始化selectedRow状态，用于存储选中的行数据
    const [selectedRow, setSelectedRow] = useState<GetMemcpyOverallResultItem | null>();
    const { t } = useTranslation();

    return props.card !== undefined && props.card.cardId !== ''
        ? view({
            mainContainer: session !== undefined
                ? <MemcpyOverallMetricsTable {...props} selectedRow={selectedRow} setSelectedRow={setSelectedRow}
                    session={session}/>
                : <></>,
            draggableContainer: <StyledMoreCard
                className="moreContainer"
                title={t('Details')}
                bordered={false}>
                <ChartErrorBoundary className={'more-error'}>
                    <MoreContainer>
                        {session && <MemcpyOverallMetricsMoreTable {...props} session={session} selectedRow={selectedRow}/>}
                    </MoreContainer>
                </ChartErrorBoundary>
            </StyledMoreCard>,
            id: 'overallMetrics',
        })
        : <div style={{ display: 'flex', height: '100%' }}>
            <StyledEmpty style={{ margin: 'auto' }}/>
        </div>;
});
