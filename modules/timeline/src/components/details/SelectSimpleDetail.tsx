/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import type { MoreTableProps, TableViewProps } from './types';
import { useDetailUpdater, useMoreUpdater } from './hooks';
import { selectRow } from './utils';
import type { CommonStateProto, TabProto } from './base/Tabs';
import { ResizeTable } from 'ascend-resize';
import { getAutoKey } from '../../utils/dataAutoKey';
import { SorterResult } from 'antd/lib/table/interface';

const TABLE_HEAD_HEIGHT = 35;
const TABLE_SUMMARY_HEIGHT = 30;
const TABLE_MIN_WIDTH = 900;

export const SelectSimpleTabularDetail = observer(<T extends CommonStateProto>(
    { session, height, detail, tabState, commonState, depsList, summaryBuilder }: TableViewProps<TabProto, T>) => {
    useEffect(() => {
        runInAction(() => {
            session.selectedDetailKeys = [];
            session.selectedDetails = [];
            session.selectedMultiSlice = '';
        });
    }, [session.selectedRange]);
    const state = useDetailUpdater(session, detail, tabState, depsList);
    const unit = session.selectedUnits[0];
    const [dataSource, setDataSource] = useState(state.dataSource);
    useEffect(() => {
        setDataSource(state.dataSource);
    }, [state.dataSource]);
    // 新增Summary(Totals)行
    const summary = (): React.ReactNode => (summaryBuilder === undefined) ? undefined : summaryBuilder(state, dataSource);
    return <ResizeTable {...state} summary={summary} dataSource={dataSource}
        scroll={{ y: height - TABLE_HEAD_HEIGHT - TABLE_SUMMARY_HEIGHT, x: TABLE_MIN_WIDTH }} virtual
        rowClassName={(row): string => {
            return session.selectedDetailKeys[0] === getAutoKey(row) ? 'selected-row' : 'click-able';
        }}
        showSorterTooltip={false}
        expandable={{ showExpandColumn: false }}
        onRow={(row): React.HTMLAttributes<any> => ({
            onClick: async (): Promise<void> => {
                detail?.clickCallback?.({ row, session, detail, unit, commonState });
                selectRow(row, session, state);
                if (detail?.fetchMoreData !== undefined && detail.more?.field !== undefined) {
                    row[detail.more?.field] = await detail?.fetchMoreData(session, row).catch(() => []);
                }
            },
            onDoubleClick: (): void => {
                selectRow(row, session, state);
                detail?.doubleClickCallback?.({ row, session, detail, unit, commonState });
            },
            onMouseEnter: (): void => {
                detail?.mouseEnterCallback?.({ session, row });
            },
            onMouseLeave: (): void => {
                detail?.mouseLeaveCallback?.({ session, row });
            },
        })}
        onChange={(pagination, filters, sorter, { currentDataSource }): void => {
            let filteredData = [...state.dataSource];
            // 筛选
            Object.keys(filters).forEach(key => {
                const filterValues = filters[key];
                if (filterValues !== null) {
                    const columnFilter = state.columns.find(col => col.key === key)?.onFilter;
                    if (columnFilter) {
                        filteredData = filteredData.filter(item => filterValues.some(value => columnFilter(value, item)));
                    }
                }
            });

            // 排序
            const { columnKey, order } = sorter as SorterResult<Record<string, unknown>>;
            if (order) {
                const columnSorter = state.columns.find(col => col.key === columnKey)?.sorter;
                filteredData = filteredData.sort((a, b) => {
                    if (typeof columnSorter === 'function') {
                        const result = columnSorter(a, b);
                        return order === 'ascend' ? result : -result;
                    }
                    return 0;
                });
            }

            setDataSource(filteredData);
        }}
    />;
});

export const SelectSimpleTabularMore = observer(({ more, height, session }: MoreTableProps) => {
    const state = useMoreUpdater(session, more);
    return <ResizeTable {...state}
        expandable={{ showExpandColumn: false }}/>;
});
