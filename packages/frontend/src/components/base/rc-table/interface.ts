import { TooltipProps } from 'antd';
import * as React from 'react';
import {
    ColumnType as RcColumnType, ExpandableConfig, GetRowKey, Key, TriggerEventHandler
} from './types';

export type { GetRowKey, ExpandableConfig, Key };

export interface TableLocale {
    filterTitle?: string;
    filterConfirm?: React.ReactNode;
    filterReset?: React.ReactNode;
    filterEmptyText?: React.ReactNode;
    filterCheckall?: React.ReactNode;
    filterSearchPlaceholder?: string;
    emptyText?: React.ReactNode | (() => React.ReactNode);
    sortTitle?: string;
    expand?: string;
    collapse?: string;
    triggerDesc?: string;
    triggerAsc?: string;
    cancelSort?: string;
}

export type SortOrder = 'descend' | 'ascend' | null;

const TableActions = [ 'sort', 'filter' ] as const;
export type TableAction = typeof TableActions[number];

export type CompareFn<T> = (a: T, b: T, sortOrder?: SortOrder) => number;

export interface ColumnFilterItem {
    text: React.ReactNode;
    value: string | number | boolean;
    children?: ColumnFilterItem[];
}

export interface ColumnTitleProps<RecordType> {
    sortColumns?: Array<{ column: ColumnType<RecordType>; order?: SortOrder }>;
    filters?: Record<string, string[]>;
}

export type ColumnTitle<RecordType> =
    | React.ReactNode
    | ((props: ColumnTitleProps<RecordType>) => React.ReactNode);

export type FilterValue = (Key | boolean)[];
export type FilterKey = Key[] | null;
export type FilterSearchType = boolean | ((input: string, record: {}) => boolean);
export interface FilterConfirmProps {
    closeDropdown: boolean;
}

export interface FilterDropdownProps {
    prefixCls: string;
    setSelectedKeys: (selectedKeys: React.Key[]) => void;
    selectedKeys: React.Key[];
    confirm: (param?: FilterConfirmProps) => void;
    clearFilters?: () => void;
    filters?: ColumnFilterItem[];
    visible: boolean;
}

export interface ColumnType<RecordType> extends RcColumnType<RecordType> {
    // Sorter
    sorter?:
        | boolean
        | CompareFn<RecordType>
        | {
            compare?: CompareFn<RecordType>;
            /** Config multiple sorter order priority */
            multiple?: number;
        };
    sortOrder?: SortOrder;
    defaultSortOrder?: SortOrder;
    sortDirections?: SortOrder[];
    showSorterTooltip?: boolean | TooltipProps;

    // Filter
    filtered?: boolean;
    filters?: ColumnFilterItem[];
    filterDropdown?: React.ReactNode | ((props: FilterDropdownProps) => React.ReactNode);
    filterMultiple?: boolean;
    filteredValue?: FilterValue | null;
    defaultFilteredValue?: FilterValue | null;
    filterIcon?: React.ReactNode | ((filtered: boolean) => React.ReactNode);
    filterMode?: 'menu' | 'tree';
    filterSearch?: boolean;
    onFilter?: (value: string | number | boolean, record: RecordType) => boolean;
    filterDropdownVisible?: boolean;
    onFilterDropdownVisibleChange?: (visible: boolean) => void;
}

export type ColumnsType<RecordType = unknown> = Array<ColumnType<RecordType>>;

export interface TableRowSelection<T> {
    selectedRowKeys?: Key[];
    defaultSelectedRowKeys?: Key[];
    onChange?: (selectedRowKeys: Key[], selectedRows: T[]) => void;
}

export type TransformColumns<RecordType> = (columns: ColumnsType<RecordType>) => ColumnsType<RecordType>;

export interface TableCurrentDataSource<RecordType> {
    currentDataSource: RecordType[];
    action: TableAction;
}

export interface SorterResult<RecordType> {
    column?: ColumnType<RecordType>;
    order?: SortOrder;
    field?: Key | readonly Key[];
    columnKey?: Key;
}

export type GetPopupContainer = (triggerNode: HTMLElement) => HTMLElement;

export type TableHandle<RecordType = unknown> = {
    scrollTo: (node: RecordType) => void;
    appendExpandedKeys: (res: Key[]) => void;
    selectFirstRoot: () => void;
};
