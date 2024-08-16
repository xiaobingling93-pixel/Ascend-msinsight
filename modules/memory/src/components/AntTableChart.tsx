/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import type { TableProps } from 'antd/es/table';
import type { SorterResult } from 'antd/lib/table/interface';
import * as React from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import i18n from 'ascend-i18n';
import type { MemoryTable, MemoryTableColumn, OperatorDetail } from '../entity/memory';
import { ResizeTable } from 'ascend-resize';

interface IProps {
    tableData: MemoryTable;
    sortColumn?: string;
    onRowSelected?: (record?: OperatorDetail, rowIndex?: number) => void;
    current: number;
    pageSize: number;
    onCurrentChange: (record: number) => void;
    onPageSizeChange: (record: number) => void;
    onOrderChange: (order: string | undefined) => void;
    onOrderByChange: (orderBy: string) => void;
    total: number;
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

const getTableColumns = function (
    columns: MemoryTableColumn[],
    sort: string | undefined,
    t: TFunction,
): any {
    return columns.map((col: MemoryTableColumn, index) => {
        return {
            dataIndex: col.key,
            key: col.key,
            title: t(col.name, { defaultValue: col.name, keyPrefix: 'tableHead' }),
            sorter: true,
            ellipsis: true,
            showSorterTooltip: t(col.name, { keyPrefix: 'tableHeadTooltip', defaultValue: '' }) === '' ? true : { title: t(col.name, { keyPrefix: 'tableHeadTooltip' }) },
        };
    });
};

// eslint-disable-next-line max-lines-per-function
export const AntTableChart: React.FC<IProps> = (props) => {
    const { t } = useTranslation('memory');
    const {
        tableData, sortColumn, onRowSelected, current, pageSize,
        onCurrentChange, onPageSizeChange, total, onOrderChange, onOrderByChange,
    } = props;

    const columns = React.useMemo(
        () => getTableColumns(tableData.columns, sortColumn, t),
        [tableData.columns, sortColumn, t],
    );

    // key is used to reset the Table state (page and sort) if the columns change
    const key = React.useMemo(() => `${Math.random()}`, [tableData.columns]);

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
                onRowSelected?.(undefined, undefined);
            },
        };
    };

    return (
        <ResizeTable
            columns={columns}
            dataSource={tableData.rows.map((item, index) => { return { ...item, key: `${item.name}_${index}` }; })}
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
        />
    );
};
