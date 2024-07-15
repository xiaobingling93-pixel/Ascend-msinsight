/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import React from 'react';
import { RefCell as Cell } from './Cell';
import { useTableContext } from './contexts';
import type { CellType, ColumnsType, ColumnType, GetComponentProps, StickyOffsets } from './types';
import { getCellFixedInfo } from './utils/fixUtil';
import { getColumnsKey } from './utils/valueUtil';

interface RowProps<RecordType> {
    cells: ReadonlyArray<CellType<RecordType>>;
    stickyOffsets: StickyOffsets;
    columns: ReadonlyArray<ColumnType<RecordType>>;
    onHeaderRow?: GetComponentProps<ReadonlyArray<ColumnType<RecordType>>>;
    index: number;
}

function HeaderRow<RecordType>({
    cells,
    stickyOffsets,
    columns,
    onHeaderRow,
    index,
}: RowProps<RecordType>): JSX.Element {
    const { prefixCls } = useTableContext();

    let rowProps: React.HTMLAttributes<HTMLElement> = {};
    if (onHeaderRow) {
        rowProps = onHeaderRow(
            cells.map(cell => cell.column as ColumnType<RecordType>),
            index,
        );
    }

    const columnsKey = getColumnsKey(cells.map(cell => cell.column as ColumnType<RecordType>));

    return (
        <tr {...rowProps}>
            {cells.map((cell: CellType<RecordType>, cellIndex) => {
                const { column } = cell;
                const fixedInfo = getCellFixedInfo(
                    cell.colStart as number,
                    cell.colEnd as number,
                    columns,
                    stickyOffsets,
                );

                let additionalProps: React.HTMLAttributes<HTMLElement> = column?.onHeaderCell?.(column) ?? {};

                return (
                    <Cell
                        {...cell}
                        ellipsis={column?.ellipsis}
                        align={column?.align}
                        component={'th'}
                        prefixCls={prefixCls}
                        key={columnsKey[cellIndex]}
                        {...fixedInfo}
                        additionalProps={additionalProps}
                        rowType="header"
                    />
                );
            })}
        </tr>
    );
}

function parseHeaderRows<RecordType>(columns: ColumnsType<RecordType>): Array<CellType<RecordType>> {
    const rows: Array<CellType<RecordType>> = [];
    let currentColIndex = 0;
    columns.forEach(column => {
        const cell: CellType<RecordType> = {
            key: column.key,
            className: column.className ?? '',
            children: column.title,
            column,
            colStart: currentColIndex, // inclusive
        };
        const colSpan = column.colSpan ?? 1;
        cell.colSpan = colSpan;
        cell.colEnd = currentColIndex + colSpan - 1;
        rows.push(cell);
        currentColIndex += colSpan;
    });
    return rows;
}

export interface HeaderProps<RecordType> {
    columns: ColumnsType<RecordType>;
    stickyOffsets: StickyOffsets;
    onHeaderRow?: GetComponentProps<ReadonlyArray<ColumnType<RecordType>>>;
}

export function Header<RecordType>({
    stickyOffsets,
    columns,
    onHeaderRow,
}: HeaderProps<RecordType>): React.ReactElement {
    const { prefixCls } = useTableContext();
    const row: Array<CellType<RecordType>> = React.useMemo(() => parseHeaderRows(columns), [columns]);

    return (
        <thead className={`${prefixCls}-thead`}>
            <HeaderRow
                key={0}
                columns={columns}
                cells={row}
                stickyOffsets={stickyOffsets}
                onHeaderRow={onHeaderRow}
                index={0}
            />
        </thead>
    );
}
