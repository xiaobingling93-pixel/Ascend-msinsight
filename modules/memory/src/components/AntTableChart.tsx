/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { Table } from 'antd';
import type { TableProps } from 'antd/es/table';
import { SorterResult } from 'antd/lib/table/interface';
import * as React from 'react';
import { MemoryTable, MemoryTableColumn, OperatorDetail } from '../entity/memory';

interface IProps {
    tableData: MemoryTable;
    sortColumn?: string;
    onRowSelected?: (record?: object, rowIndex?: number) => void;
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
}

const orderByColName: IColName = {
    name: 'name',
    size: 'size',
    allocationTime: 'allocation_time',
    releaseTime: 'release_time',
    duration: 'duration',
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
            width: index === 0 ? '40%' : '15%',
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
        [ tableData.columns, sortColumn ],
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

    const onRow = (record: object, rowIndex?: number): React.HTMLAttributes<any> => {
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
        <Table
            size="small"
            bordered
            columns={columns}
            dataSource={tableData.rows.map((item, index) => { return { ...item, key: `${item.name}_${index}` }; })}
            onChange={onTableChange}
            pagination={{
                current,
                pageSize,
                pageSizeOptions: [ '10', '20', '30', '50', '100' ],
                onChange,
                total,
                showTotal: total => `Total ${total} items`,
                showQuickJumper: true,
            }}
            rowClassName="memory-ant-table-row"
            key={key}
            onRow={onRow}
        />
    );
};
