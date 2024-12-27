/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import type { TableProps } from 'antd/es/table';
import type { SorterResult, TablePaginationConfig } from 'antd/lib/table/interface';
import React, { useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import i18n from 'ascend-i18n';
import type { ComponentMemory, GetTableDataResponse, MemoryTable, MemoryTableColumn, OperatorDetail, OrderPageInfo, RenderExpandRecord } from '../entity/memory';
import { ResizeTable } from 'ascend-resize';
import { Button, Spin } from 'ascend-components';
import { DownOutlined } from '@ant-design/icons';
import type { TableColumnsType, MenuProps } from 'antd';
import { Dropdown } from 'antd';
import { type Theme, useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { useRootStore } from '../context/context';
import { fetchOperatorPosition, fetchTableDataByComponent } from '../utils/RequestUtils';
import type { Session } from '../entity/session';
import { handleOperatorDetails } from './MemoryDetailTable';
import connector from '../connection';

interface IProps {
    tableData: MemoryTable;
    onRowSelected?: (record?: OperatorDetail, rowIndex?: number) => void;
    current: number;
    pageSize: number;
    onCurrentChange: (newCurrent: number) => void;
    onPageSizeChange: (newPageSize: number) => void;
    onOrderChange: (order: string | undefined) => void;
    onOrderByChange: (orderBy: string) => void;
    total: number;
    isCompare: boolean;
    rankId: string;
}

interface IColName {
    deviceId: string;
    name: string;
    opName: string;
    nodeIndexStart: string;
    nodeIndexEnd: string;
    size: string;
    allocationTime: string;
    releaseTime: string;
    duration: string;
    activeReleaseTime: string;
    activeDuration: string;
    allocationAllocated: string;
    allocationReserved: string;
    allocationActive: string;
    releaseAllocated: string;
    releaseReserved: string;
    releaseActive: string;
    streamId: string;
}

const orderByColName: IColName = {
    deviceId: 'device_id',
    name: 'name',
    opName: 'op_name',
    nodeIndexStart: 'node_index_start',
    nodeIndexEnd: 'node_index_end',
    size: 'size',
    allocationTime: 'allocation_time',
    releaseTime: 'release_time',
    duration: 'duration',
    activeReleaseTime: 'active_release_time',
    activeDuration: 'active_duration',
    allocationAllocated: 'allocation_allocated',
    allocationReserved: 'allocation_reserve',
    allocationActive: 'allocation_active',
    releaseAllocated: 'release_allocated',
    releaseReserved: 'release_reserve',
    releaseActive: 'release_active',
    streamId: 'stream',
};

const CompareDiv = styled.div`
    width: 100%;
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
    word-break: keep-all;
`;

const getCompareRows = (data: string | number, theme: Theme): JSX.Element | number | string => {
    const dataNum = Number(data);
    if (isNaN(dataNum)) {
        return data;
    }
    return <CompareDiv style={{ color: dataNum >= 0 ? theme.successColor : theme.dangerColor }} title={`${data}`}>{data}</CompareDiv>;
};

const renderExpandColomn = (
    record: RenderExpandRecord,
    t: TFunction,
    setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>,
): JSX.Element => {
    return record.source === t('Difference')
        ? (<Button type="link"
            onClick={(): void => {
                setExpandedKeys((pre: any) => {
                    const list = [...pre];
                    const keyIndex = list.indexOf(record.key);
                    if (keyIndex === -1) {
                        list.push(record.key);
                    } else {
                        list.splice(keyIndex, 1);
                    }
                    return list;
                });
            }}>{t('SeeMore', { ns: 'buttonText' })}<DownOutlined /></Button>)
        : <></>;
};

const getTableColumns = function (
    columns: MemoryTableColumn[],
    theme: Theme,
    t: TFunction,
    isCompare: boolean,
    setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>,
): TableColumnsType<RenderExpandRecord> {
    const dataColumns: TableColumnsType<RenderExpandRecord> = columns.map((col: MemoryTableColumn) => {
        return {
            dataIndex: col.key,
            key: col.key,
            title: t(col.name, { defaultValue: col.name, keyPrefix: 'tableHead' }),
            sorter: true,
            ellipsis: true,
            showSorterTooltip: t(col.name, { keyPrefix: 'tableHeadTooltip', defaultValue: '' }) === ''
                ? true
                : { title: t(col.name, { keyPrefix: 'tableHeadTooltip' }) },
            render: (data: string | number, record: RenderExpandRecord) =>
                (isCompare && record.source === t('Difference')) ? getCompareRows(data, theme) : data,
        };
    });
    if (isCompare) {
        dataColumns.push({
            key: 'action',
            title: t('Details', { keyPrefix: 'tableHead' }),
            sorter: false,
            fixed: 'right',
            render: (record: RenderExpandRecord): JSX.Element => {
                return renderExpandColomn(record, t, setExpandedKeys);
            },
        });
    }
    return dataColumns;
};

let selectedRecord: OperatorDetail | undefined;

async function redirectToTimeline(record: OperatorDetail, rankId: string): Promise<void> {
    const res = await fetchOperatorPosition({
        id: record.id,
        name: record.name,
        rankId,
    });
    connector.send({
        event: 'switchModule',
        body: {
            switchTo: 'timeline',
            toModuleEvent: 'locateUnit',
            params: {
                ...res,
            },
        },
    });
}

// eslint-disable-next-line max-lines-per-function
export const AntTableChart: React.FC<IProps> = (props) => {
    const defaultDataSource = [{}] as OperatorDetail[]; // 防止antd4 table组件报ResizeObserver loop错误
    const { t } = useTranslation('memory');
    const {
        tableData, onRowSelected, current, pageSize,
        onCurrentChange, onPageSizeChange, total, onOrderChange, onOrderByChange, isCompare, rankId,
    } = props;
    const [open, setOpen] = useState<boolean>(false);
    const [shouldBlockMouseLeave, setShouldBlockMouseLeave] = useState<boolean>(false);
    const theme = useTheme();
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);

    const columns = useMemo(
        () => getTableColumns(tableData.columns, theme, t, isCompare, setExpandedKeys),
        [tableData.columns, t, isCompare],
    );

    const useMenuItems = (): MenuProps['items'] => {
        if (selectedRecord === undefined || !open) {
            return [];
        }
        return [
            {
                label: t('Find in Timeline'),
                key: 'findInTimeline',
                onClick: (): void => {
                    if (selectedRecord !== undefined) {
                        redirectToTimeline(selectedRecord, rankId);
                    }
                },
            },
        ];
    };
    const items = useMenuItems();

    // key is used to reset the Table state (page and sort) if the columns change
    const key = useMemo(() => `${Math.random()}`, [tableData.columns]);

    const onChange = (newCurrent: number, size: number): void => {
        onCurrentChange(newCurrent);
        onPageSizeChange(size);
    };

    const onTableChange: TableProps<OperatorDetail>['onChange'] = (pagination, filter,
        sorter: SorterResult<OperatorDetail> | Array<SorterResult<OperatorDetail>>) => {
        if ((sorter as SorterResult<OperatorDetail>).order) {
            const orderByCol = `${(sorter as SorterResult<OperatorDetail>).field}`;
            onOrderChange((sorter as SorterResult<OperatorDetail>).order as string);
            onOrderByChange(orderByColName[orderByCol as keyof IColName]);
        } else {
            onOrderChange(undefined);
        }
    };

    const onRow = (record: OperatorDetail, rowIndex?: number): React.HTMLAttributes<any> => {
        return {
            onMouseEnter: (event: any): void => {
                onRowSelected?.(record, rowIndex);
            },
            onMouseLeave: (event: any): void => {
                if (shouldBlockMouseLeave) {
                    setShouldBlockMouseLeave(false);
                    return;
                }
                setOpen(false);
                onRowSelected?.(undefined, undefined);
            },
            onContextMenu: (event: any): void => {
                event.preventDefault(); // 阻止默认的右键菜单
                selectedRecord = record;
                setOpen(true);
                setShouldBlockMouseLeave(true);
            },
        };
    };

    useEffect(() => {
        setExpandedKeys([]);
    }, [JSON.stringify(tableData), current, pageSize]);

    return (
        <Dropdown menu={{ items }} trigger={['contextMenu']}>
            <div>
                <ResizeTable
                    columns={columns as TableColumnsType<OperatorDetail>}
                    dataSource={tableData.rows.length === 0 ? defaultDataSource : tableData.rows.map((item, index) => ({ ...item, key: `${item.name}_${index}` }))}
                    onChange={onTableChange}
                    scroll={{
                        x: 150 * columns.length,
                    }}
                    pagination={{
                        current,
                        pageSize,
                        pageSizeOptions: ['10', '20', '30', '50', '100'],
                        onChange,
                        total,
                        showTotal: (totalNum: number): string => i18n.t('PaginationTotal', { total: totalNum }),
                        showQuickJumper: true,
                    }}
                    rowClassName="memory-ant-table-row"
                    key={key}
                    onRow={onRow}
                    expandable={{
                        expandIcon: () => <></>,
                        expandedRowKeys,
                    }}
                />
            </div>
        </Dropdown>
    );
};

let pagination: TablePaginationConfig = {
    defaultCurrent: 1,
    defaultPageSize: 10,
    pageSizeOptions: ['10', '20', '30', '50', '100'],
    showTotal: (total: number): string => i18n.t('PaginationTotal', { total }),
    showQuickJumper: true,
};

export const TableByComponent = ({ session }: { session: Session }): JSX.Element => {
    const { t } = useTranslation('memory');
    const { memoryStore } = useRootStore();
    const memorySession = memoryStore.activeSession;
    const [response, setResponse] = useState<GetTableDataResponse>({ totalNum: 0, columnAttr: [], componentDetail: [] });
    const [columns, setColumns] = useState<TableColumnsType<ComponentMemory>>([]);
    const [tableData, setTableData] = useState<ComponentMemory[]>([]);
    const [expandedRowKeys, setExpandedKeys] = useState<string[]>([]);
    const [tableSpin, setTableSpin] = useState(false);
    const theme = useTheme();
    const onTableChange: TableProps<ComponentMemory>['onChange'] = (paginationValue, _filter, originSorter) => {
        const { current, pageSize } = paginationValue;
        const sorter = originSorter as SorterResult<ComponentMemory>;
        const order = sorter.field === 'source' ? undefined : sorter.order ?? undefined;
        const orderBy = order === undefined ? undefined : sorter.field as string;
        getTableData({
            currentPage: current === undefined ? 1 : current,
            pageSize: pageSize === undefined ? 10 : pageSize,
            order,
            orderBy,
        });
    };
    const getTableData = async (value: OrderPageInfo = { currentPage: 1, pageSize: 10 }): Promise<void> => {
        setTableSpin(true);
        if (memorySession === undefined || memorySession.rankIdCondition.value === undefined || memorySession.rankIdCondition.value === '') {
            setTableSpin(false);
            setTableData([]);
            return;
        }
        try {
            const res = await fetchTableDataByComponent({
                ...value,
                rankId: memorySession.rankIdCondition.value,
                isCompare: session.compareRank.isCompare,
            });
            setResponse({ ...res });
        } catch {
            setResponse({ totalNum: 0, columnAttr: [], componentDetail: [] });
        }
        setTableSpin(false);
    };

    useEffect(() => {
        getTableData();
    }, [memorySession?.rankIdCondition.value, session.compareRank.isCompare]);
    useEffect(() => {
        pagination = { ...pagination, total: response.totalNum };
        setColumns(getTableColumns(response.columnAttr, theme, t, session.compareRank.isCompare, setExpandedKeys) as TableColumnsType<ComponentMemory>);
        setTableData(handleOperatorDetails(response.componentDetail, session.compareRank.isCompare, t));
    }, [response, session.language]);

    return <Spin spinning={tableSpin} tip={t('Loading')}>
        <ResizeTable columns={columns} dataSource={tableData.map((item, index) => ({ ...item, key: `${item.name}_${index}` }))} pagination={pagination} onChange={onTableChange}
            expandable={{ expandIcon: () => <></>, expandedRowKeys }} />
    </Spin>;
};
