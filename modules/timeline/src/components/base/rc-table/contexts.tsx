/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import React from 'react';
import type { ColumnType, DefaultRecordType, RenderExpandIcon, RowClassName, TableLayout, TriggerEventHandler } from './types';
import type { FixedInfo } from './utils/fixUtil';

interface ResizeContextProps {
    onColumnResize: (columnKey: React.Key, width: number) => void;
}

export const ResizeContext = React.createContext<ResizeContextProps | null>(null);

export const useResizeContext = (): ResizeContextProps => {
    const ctx = React.useContext(ResizeContext);
    if (!ctx) {
        throw new Error('No ResizeContext provided');
    }
    return ctx;
};

export interface TableContextProps {
    // Table context
    prefixCls: string;
    scrollbarSize: number;
    fixedInfoList: readonly FixedInfo[];
}

export const TableContext = React.createContext<TableContextProps | null>(null);

export const useTableContext = (): TableContextProps => {
    const ctx = React.useContext(TableContext);
    if (!ctx) {
        throw new Error('No TableContext provided');
    }
    return ctx;
};

export interface BodyContextProps<RecordType = DefaultRecordType> {
    rowClassName?: string | RowClassName<RecordType>;
    flattenColumns: ReadonlyArray<ColumnType<RecordType>>;
    tableLayout?: TableLayout;
    indentSize: number;
    expandIcon?: RenderExpandIcon<RecordType>;
    onTriggerExpand: TriggerEventHandler<RecordType>;
    expandIconColumnIndex?: number;
}

export const BodyContext = React.createContext<BodyContextProps<any> | null>(null);

export const useBodyContext = (): BodyContextProps<any> => {
    const ctx = React.useContext(BodyContext);
    if (!ctx) {
        throw new Error('No BodyContext provided!');
    }
    return ctx;
};

export interface PerfRecord {
    renderWithProps: boolean;
}
export const PerfContext = React.createContext<PerfRecord>({
    renderWithProps: false,
});
