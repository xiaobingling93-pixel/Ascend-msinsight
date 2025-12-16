/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
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
import React, { ReactNode, useEffect, useMemo, useState } from 'react';
import { Table } from 'antd';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import { TableState, TableViewProps } from './types';
import { useDetailUpdater } from './hooks';
import type { CommonStateProto, TabProto } from './base/Tabs';
import { ResizeTable } from '@insight/lib/resize';
import { AutoKey, getAutoKey } from '../../utils/dataAutoKey';
import { SorterResult } from 'antd/lib/table/interface';

const TABLE_HEAD_HEIGHT = 35;
const TABLE_SUMMARY_HEIGHT = 30;
const TABLE_MIN_WIDTH = 1260;

const generateSummary = (state: TableState, dataSource: Array<AutoKey<object>>): ReactNode => (
    <Table.Summary fixed={'top'}>
        <Table.Summary.Row>
            { state.columns.map((column, index) => (
                <Table.Summary.Cell key={column.key} index={index}>
                    {column.summary?.(dataSource)}
                </Table.Summary.Cell>
            ))}
        </Table.Summary.Row>
    </Table.Summary>
);

export const SelectSimpleTabularDetail = observer(<T extends CommonStateProto>(
    { session, height, detail, tabState, commonState, depsList }: TableViewProps<TabProto, T>) => {
    useEffect(() => {
        runInAction(() => {
            session.selectedMultiSlice = '';
        });
    }, [session.selectedRange]);
    const state = useDetailUpdater(session, detail, tabState, depsList);
    const unit = session.selectedUnits[0];
    const [dataSource, setDataSource] = useState(state.dataSource);
    const [selectedKey, setSelectedKey] = useState<string | null>(null);
    const scrollY = useMemo(() => {
        let val = height - TABLE_HEAD_HEIGHT;
        if (dataSource?.length) {
            val = val - TABLE_SUMMARY_HEIGHT;
        }
        return val;
    }, [height, dataSource?.length]);
    useEffect(() => {
        setDataSource(state.dataSource);
    }, [state.dataSource]);
    // 新增Summary(Totals)行
    const summary = (): React.ReactNode => dataSource?.length ? generateSummary(state, dataSource) : undefined;
    return <ResizeTable className={'table-slice-list'} {...state} summary={summary} dataSource={dataSource} allowCopy
        scroll={{ y: scrollY, x: TABLE_MIN_WIDTH }} virtual
        rowClassName={(row): string => {
            return selectedKey !== null && selectedKey === getAutoKey(row) ? 'selected-row' : 'click-able';
        }}
        showSorterTooltip={false}
        expandable={{ showExpandColumn: false }}
        onRow={(row): React.HTMLAttributes<any> => ({
            onClick: async (): Promise<void> => {
                detail?.clickCallback?.({ row, session, detail, unit, commonState });
                setSelectedKey(state.rowKey?.(row) ?? getAutoKey(row));
                if (detail?.fetchMoreData !== undefined && detail.more?.field !== undefined) {
                    row[detail.more?.field] = await detail?.fetchMoreData(session, row).catch(() => []);
                }
            },
            onDoubleClick: (): void => {
                setSelectedKey(state.rowKey?.(row) ?? getAutoKey(row));
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
