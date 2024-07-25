/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { TooltipProps } from 'antd';
import { Spin } from 'lib/components';
import useFilter from 'antd/lib/table/hooks/useFilter';
import useSorter, { getSortData } from 'antd/lib/table/hooks/useSorter';
import useTitleColumns from 'antd/lib/table/hooks/useTitleColumns';
import classNames from 'classnames';
import useMergedState from 'rc-util/lib/hooks/useMergedState';
import * as React from 'react';
import BaseTable, { INSIGHT_TABLE_PREFIX, type TableProps as BaseTableProps } from './BaseTable';
import type {
    ColumnsType, GetPopupContainer, GetRowKey, Key, SortOrder, TableHandle, TableLocale, TableRowSelection
} from './interface';
import { customizeFilterData } from './utils/filterUtil';
import type { ColumnsType as AntdColumnsType } from 'antd/lib/table/interface';
import type { DefaultRecordType, ColumnsType as TypesColumnsType } from './types';
import type { EmotionJSX } from '@emotion/react/types/jsx-namespace';

export type { ColumnsType };

const EMPTY_LIST: React.Key[] = [];

interface DefaultExpandIconProps<RecordType> {
    prefixCls?: string;
    onExpand: (record: RecordType, e: React.MouseEvent<HTMLElement>) => void;
    record: RecordType;
    expanded: boolean;
    expandable: boolean;
}

function expandIconFor(locale: TableLocale) {
    return function expandIcon<RecordType>({
        prefixCls,
        onExpand,
        record,
        expanded,
        expandable,
    }: DefaultExpandIconProps<RecordType>): EmotionJSX.Element {
        const iconPrefix = `${prefixCls}-row-expand-icon`;

        return (
            <button
                type="button"
                onClick={(e): void => {
                    onExpand(record, e);
                    e.stopPropagation();
                }}
                className={classNames(iconPrefix, {
                    [`${iconPrefix}-spaced`]: !expandable,
                    [`${iconPrefix}-expanded`]: expandable && expanded,
                    [`${iconPrefix}-collapsed`]: expandable && !expanded,
                })}
                aria-label={expanded ? locale.collapse : locale.expand}
                aria-expanded={expanded}
            />
        );
    };
}

const defaultLocale: TableLocale = {
    cancelSort: 'Click to cancel sorting',
    collapse: 'Collapse row',
    emptyText: 'No Data',
    expand: 'Expand row',
    filterCheckall: 'Select all items',
    filterConfirm: 'OK',
    filterEmptyText: 'No filters',
    filterReset: 'Reset',
    filterSearchPlaceholder: 'Search in filters',
    filterTitle: 'Filter menu',
    sortTitle: 'Sort',
    triggerAsc: 'Click to sort ascending',
    triggerDesc: 'Click to sort descending',
};

function useSelection<RecordType>(rowSelection?: TableRowSelection<RecordType>): Set<Key> {
    const {
        selectedRowKeys,
        defaultSelectedRowKeys,
    } = rowSelection ?? {};
    const [mergedSelectedKeys, setMergedSelectedKeys] = useMergedState(
        selectedRowKeys ?? defaultSelectedRowKeys ?? EMPTY_LIST,
        { value: selectedRowKeys },
    );
    React.useEffect(() => {
        if (!rowSelection) {
            setMergedSelectedKeys(EMPTY_LIST);
        }
    }, [!!rowSelection]);
    return new Set(mergedSelectedKeys);
}

export interface TableProps<RecordType>
    extends Omit<
        BaseTableProps<RecordType>,
        | 'transformColumns'
        | 'data'
        | 'columns'
        | 'emptyText'
    > {
    dropdownPrefixCls?: string;
    dataSource?: BaseTableProps<RecordType>['data'];
    columns: ColumnsType<RecordType>;
    isLoading?: boolean;
    locale?: TableLocale;
    rowSelection?: TableRowSelection<RecordType>;

    getPopupContainer?: GetPopupContainer;
    sortDirections?: SortOrder[];
    showSorterTooltip?: boolean | TooltipProps;
}

function InternalTable<RecordType extends DefaultRecordType>(
    props: TableProps<RecordType>,
    ref: React.MutableRefObject<TableHandle>,
): EmotionJSX.Element {
    const {
        prefixCls: customizePrefixCls,
        className,
        dropdownPrefixCls: customizeDropdownPrefixCls,
        dataSource = [],
        rowHeight,
        rowSelection,
        rowKey,
        rowClassName,
        columns,
        getPopupContainer,
        isLoading = false,
        expandable,
        sortDirections = ['descend', 'ascend'],
        locale,
        showSorterTooltip = true,
    } = props;

    const tableLocale = { ...defaultLocale, ...locale };
    const prefixCls = customizePrefixCls ?? INSIGHT_TABLE_PREFIX;
    const mergedExpandable = { ...expandable };
    const { childrenColumnName = 'children' } = mergedExpandable;

    const getRowKey = React.useMemo<GetRowKey<RecordType>>(() => {
        if (typeof rowKey === 'function') {
            return rowKey;
        }
        return (record: RecordType): Key => (record as any)?.[rowKey as string];
    }, [rowKey]);

    // ============================ Sort ============================
    const [transformSorterColumns, sortStates, sorterTitleProps] = useSorter<RecordType>({
        prefixCls,
        mergedColumns: columns as AntdColumnsType<RecordType>,
        sortDirections,
        tableLocale,
        showSorterTooltip,
        onSorterChange: () => {},
    });
    const sortedData = React.useMemo(
        () => getSortData(dataSource, sortStates, childrenColumnName),
        [dataSource, sortStates],
    );

    // ============================ Filter ============================
    const dropdownPrefixCls = customizeDropdownPrefixCls ?? 'ant-dropdown';
    const [transformFilterColumns, filterStates] = useFilter<RecordType>({
        prefixCls,
        locale: tableLocale,
        dropdownPrefixCls,
        mergedColumns: columns as AntdColumnsType<RecordType>,
        getPopupContainer,
        onFilterChange: () => {},
    });
    // 添加useMemo,目的是在过滤树形结构时，避免重复计算引起卡顿的问题。
    const pageData = React.useMemo(() => customizeFilterData(sortedData, filterStates), [filterStates, sortedData]);

    const [transformTitleColumns] = useTitleColumns(sorterTitleProps);
    const transformColumns = React.useCallback((innerColumns: ColumnsType<RecordType>): ColumnsType<RecordType> =>
        transformTitleColumns(transformFilterColumns(transformSorterColumns(innerColumns as AntdColumnsType<RecordType>))) as ColumnsType<RecordType>,
        [transformSorterColumns, transformFilterColumns],
    );

    const selectedKeySet = useSelection<RecordType>(rowSelection);

    const internalRowClassName = (record: RecordType, index: number, level: number): string =>
        classNames(
            {
                [`${prefixCls}-row-selected`]: selectedKeySet.has(getRowKey(record, index)),
            },
            typeof rowClassName === 'function' ? rowClassName(record, index, level) : rowClassName,
        );

    mergedExpandable.expandIcon = mergedExpandable.expandIcon || expandIconFor(tableLocale);

    const wrapperClassNames = classNames(`${prefixCls}-wrapper`, className);
    Spin.setDefaultIndicator(<div className='loading'/>);
    return (
        <div className={wrapperClassNames}>
            <Spin spinning={isLoading}>
                <BaseTable<RecordType>
                    {...props}
                    ref={ref}
                    columns={columns as TypesColumnsType<RecordType>}
                    expandable={mergedExpandable}
                    prefixCls={prefixCls}
                    className={classNames({
                        [`${prefixCls}-empty`]: dataSource.length === 0,
                    })}
                    data={pageData}
                    rowHeight={rowHeight}
                    rowKey={getRowKey}
                    rowClassName={internalRowClassName}
                    emptyText={locale?.emptyText ?? 'No Data'}
                    transformColumns={transformColumns}
                />
            </Spin>
        </div>
    );
}

const ForwardTable = React.forwardRef(InternalTable as React.ForwardRefRenderFunction<TableHandle, TableProps<object>>) as <RecordType extends object = any>(
    props: React.PropsWithChildren<TableProps<RecordType>> & { ref?: React.Ref<TableHandle> },
) => React.ReactElement;

type InternalTableType = typeof ForwardTable;

interface TableInterface extends InternalTableType {
    defaultProps?: Partial<TableProps<unknown>>;
}

const Table: TableInterface = ForwardTable;

Table.defaultProps = {
    rowKey: 'key',
    prefixCls: 'insight-table',
};

export default Table;
