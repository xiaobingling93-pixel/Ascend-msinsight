/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import type { TableProps } from 'antd/es/table';
import { SorterResult } from 'antd/lib/table/interface';
import * as React from 'react';
import { MemoryTable, MemoryTableColumn, OperatorDetail } from '../entity/memory';
import ResizeTable from './resize/ResizeTable';

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
    name: string;
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
    name: 'name',
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
): any {
    return columns.map((col: MemoryTableColumn, index) => {
        return {
            dataIndex: col.key,
            key: col.key,
            title: col.name,
            sorter: true,
        };
    });
};

// eslint-disable-next-line max-lines-per-function
export const AntTableChart: React.FC<IProps> = (props) => {
    const {
        tableData, sortColumn, onRowSelected, current, pageSize,
        onCurrentChange, onPageSizeChange, total, onOrderChange, onOrderByChange,
    } = props;

    const columns = React.useMemo(
        () => getTableColumns(tableData.columns, sortColumn),
        [tableData.columns, sortColumn],
    );

    // key is used to reset the Table state (page and sort) if the columns change
    const key = React.useMemo(() => `${Math.random()}`, [tableData.columns]);

    const onChange = (current: number, size: number): void => {
        onCurrentChange(current);
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
            onMouseEnter: (event: any) => {
                onRowSelected?.(record, rowIndex);
            },
            onMouseLeave: (event: any) => {
                onRowSelected?.(undefined, undefined);
            },
        };
    };

    return (
        <ResizeTable
            size="small"
            bordered
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
                showTotal: (totalNumber: number) => `Total ${totalNumber} items`,
                showQuickJumper: true,
            }}
            rowClassName="memory-ant-table-row"
            key={key}
            onRow={onRow}
        />
    );
};
